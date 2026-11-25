#include <pmgWebSocket.hpp>
// #include <algorithm>
#include <string>
#include <cstdint>
#include <map>

using namespace pmg::core;

webSocket::webSocket(httpd_handle_t server, int socketfd) : 
    _server(server), _socketfd(socketfd), _state(webSocketState::OPEN) { 
}

int32_t webSocket::getSocketFd() const { return _socketfd; }

bool webSocket::isConnected() const { return _state == webSocketState::OPEN; }

webSocketState webSocket::getReadyState() const { return _state; }

void webSocket::attach(httpd_handle_t server, int socketfd) {
    _server = server;
    _socketfd = socketfd;
    _state = webSocketState::OPEN;
}

/*** 
 * WORKS ONLY IF SERVER & WEBSOCKET ARE IN THE SAME THREAD OTHERWISE NEED TO IMPLEMENT CALLBACK
 * WITH payload destruction
 * *****/

 /**
  * 
  *

struct AsyncWsWork {
    httpd_handle_t server;
    int fd;
    std::string payload;
};

// Exécuté automatiquement dans le contexte de la tâche du serveur HTTP
static void ws_send_worker(void *arg) {
    auto *work = static_cast<AsyncWsWork *>(arg);

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = reinterpret_cast<uint8_t *>(work->payload.data());
    frame.len = work->payload.size();

    // L'envoi synchrone est sûr ici car on est dans la tâche httpd
    httpd_ws_send_frame(work->server, work->fd, &frame);

    delete work; // Libération de la mémoire une fois la trame envoyée
}

esp_err_t webSocket::send(const webSocketMessageStringType msgType, std::string_view message) {
    if (_state != webSocketState::OPEN || _server == nullptr || _socketfd < 0) {
        return ESP_ERR_INVALID_STATE;
    }

    // Allocation du contexte qui survivra jusqu'à la fin de l'envoi
    auto *work = new (std::nothrow) AsyncWsWork{_server, _socketfd, {}};
    if (!work) {
        return ESP_ERR_NO_MEM;
    }

    work->payload.reserve(message.size() + 2);
    work->payload += static_cast<char>(msgType);
    work->payload += '|';
    work->payload += message;

    // Mise en file d'attente pour exécution par la tâche serveur
    esp_err_t err = httpd_queue_work(_server, ws_send_worker, work);
    if (err != ESP_OK) {
        delete work; // Évite une fuite mémoire si la file est pleine ou le serveur arrêté
    }

    return err;
}

  * 
  */

  /**
   * 
   
   struct AsyncBinaryWork {
    httpd_handle_t server;
    int fd;
    std::vector<uint8_t> payload; // Alloué dynamiquement à la taille exacte
};

static void ws_send_binary_worker(void *arg) {
    auto *work = static_cast<AsyncBinaryWork *>(arg);

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_BINARY; // Trame binaire
    frame.payload = work->payload.data();
    frame.len = work->payload.size();

    httpd_ws_send_frame(work->server, work->fd, &frame);
    delete work;
}

esp_err_t webSocket::sendSensorData(const SensorData& data) {
    if (_state != webSocketState::OPEN || _server == nullptr || _socketfd < 0) {
        return ESP_ERR_INVALID_STATE;
    }

    // 1. Pré-calcul de la taille exacte (ex: 1 octet type + 4 octets float + ...)
    const size_t totalSize = 1 + sizeof(data.temperature) + sizeof(data.humidity);

    // 2. Allocation exacte sur le tas
    auto *work = new (std::nothrow) AsyncBinaryWork{_server, _socketfd, std::vector<uint8_t>(totalSize)};
    if (!work) {
        return ESP_ERR_NO_MEM;
    }

    // 3. Écriture directe dans le buffer du worker via BinaryWriter (sans copie)
    BinaryWriter writer(work->payload.data(), work->payload.size());
    writer.writeUint8(static_cast<uint8_t>(webSocketMessageBinaryType::SENSOR));
    writer.writeFloat(data.temperature);
    writer.writeFloat(data.humidity);

    // 4. Envoi asynchrone
    esp_err_t err = httpd_queue_work(_server, ws_send_binary_worker, work);
    if (err != ESP_OK) {
        delete work;
    }

    return err;
}

   
   */

esp_err_t webSocket::send(const uint8_t* data, size_t length) {

    if (_socketfd < 0 || _server == nullptr) return ESP_FAIL;

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_BINARY;
    frame.payload = const_cast<uint8_t*>(data);
    frame.len = length;

    return httpd_ws_send_frame_async(_server, _socketfd, &frame);
    
}

esp_err_t webSocket::send(const webSocketMessageStringType msgType, const std::string& message) {

    if (_state != webSocketState::OPEN || _server == nullptr || _socketfd < 0) return ESP_ERR_INVALID_STATE;

    std::string payload;
    payload.reserve(message.length() + 2);

    payload += static_cast<char>(msgType);
    payload += '|';
    payload += message;

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = reinterpret_cast<uint8_t *>(
        const_cast<char *>(payload.c_str())
    );
    frame.len = payload.length();

    return httpd_ws_send_frame_async(_server, _socketfd, &frame);

}

esp_err_t webSocket::ping() {
    return send(webSocketMessageStringType::PINGPONG, "PING");
}

esp_err_t webSocket::close(uint16_t code, const std::string& reason) {

    if (_state != webSocketState::OPEN || _server == nullptr || _socketfd < 0) return ESP_ERR_INVALID_STATE;

    /*
     * Control Frame 125 octets.
     *
     * CLOSE payload :
     * - 2 octets for the code error
     * - max 123 octets for reason
     */
    constexpr size_t MAX_CLOSE_PAYLOAD = 125;
    constexpr size_t MAX_REASON_LENGTH = 123;

    const size_t reasonLength = std::min(reason.length(), MAX_REASON_LENGTH);

    uint8_t payload[MAX_CLOSE_PAYLOAD];

    // Thx IA ;-)
    payload[0] = static_cast<uint8_t>((code >> 8) & 0xFF);
    payload[1] = static_cast<uint8_t>(code & 0xFF);

    if (reasonLength > 0) memcpy(payload + 2, reason.c_str(), reasonLength);

    httpd_ws_frame_t frame = {};

    frame.type = HTTPD_WS_TYPE_CLOSE;
    frame.payload = payload;
    frame.len = 2 + reasonLength;

    _state = webSocketState::CLOSING;

    /* send the closing frame (apparently it's not really async) */
    esp_err_t err = httpd_ws_send_frame_async(_server, _socketfd, &frame);

    if (err != ESP_OK) {
        _state = webSocketState::OPEN;
        return err;
    }

    /*
     * Closing frame has been sent now we can close the socket
     */
    err = httpd_sess_trigger_close(_server, _socketfd);

    if (err != ESP_OK) return err;

    return ESP_OK;

};

uint32_t webSocket::onMessage(MessageCallback callback) {
    uint32_t id = _listenerId++;
    _messageCallbacks.emplace(id, std::move(callback));
    return id;
}

uint32_t webSocket::onClose(CloseCallback callback) {
    uint32_t id = _listenerId++;
    _closeCallbacks.emplace(id, std::move(callback));
    return id;
}

uint32_t webSocket::onPong(PongCallBack callback) {
    uint32_t id = _listenerId++;
    _pongCallbacks.emplace(id, std::move(callback));
    return id;
}

void webSocket::removeMessageListener(uint32_t id) {
    _messageCallbacks.erase(id);
}

void webSocket::removeCloseListener(uint32_t id) {
    _closeCallbacks.erase(id);
}

void webSocket::removePongListener(uint32_t id) {
    _pongCallbacks.erase(id);
}

// /* what happen when a delete is called during dispatch??? */

void webSocket::dispatchMessage(const std::string& message) {
    auto it = _messageCallbacks.begin();
    while (it != _messageCallbacks.end()) {
        auto current = it++;
        current->second(_socketfd, message);
    }
}

void webSocket::dispatchPong() {
    auto it = _pongCallbacks.begin();
    while (it != _pongCallbacks.end()) {
        auto current = it++;
        current->second(_socketfd);
    }
}

void webSocket::dispatchClose(int32_t code, const std::string& reason) {

    auto it = _closeCallbacks.begin();
    while (it != _closeCallbacks.end()) {
        auto current = it++;
        current->second(_socketfd, code, reason);
    }

    _socketfd = -1;
    _server = nullptr;
    _state = webSocketState::CLOSED;

}
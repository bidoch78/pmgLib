#include <pmgWebSocket.hpp>
// #include <algorithm>
// #include <cstring>
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

esp_err_t webSocket::send(const uint8_t* data, size_t length) {

    if (_socketfd < 0 || _server == nullptr) return ESP_FAIL;

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_BINARY;
    frame.payload = const_cast<uint8_t*>(data);
    frame.len = length;

    return httpd_ws_send_frame_async(_server, _socketfd, &frame);
    
}

esp_err_t webSocket::send(const String& message) {

    if (_state != webSocketState::OPEN || _server == nullptr || _socketfd < 0) return ESP_ERR_INVALID_STATE;

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = reinterpret_cast<uint8_t *>(
        const_cast<char *>(message.c_str())
    );
    frame.len = message.length();

    return httpd_ws_send_frame_async(_server, _socketfd, &frame);

}

esp_err_t webSocket::close(uint16_t code, const String& reason) {

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

void webSocket::removeMessageListener(uint32_t id) {
    _messageCallbacks.erase(id);
}

void webSocket::removeCloseListener(uint32_t id) {
    _closeCallbacks.erase(id);
}

// /* what happen when a delete is called during dispatch??? */

void webSocket::dispatchMessage(const String& message) {
    auto it = _messageCallbacks.begin();
    while (it != _messageCallbacks.end()) {
        auto current = it++;
        current->second(_socketfd, message);
    }
}

void webSocket::dispatchClose(int32_t code, const String& reason) {

    auto it = _closeCallbacks.begin();
    while (it != _closeCallbacks.end()) {
        auto current = it++;
        current->second(_socketfd, code, reason);
    }

    _socketfd = -1;
    _server = nullptr;
    _state = webSocketState::CLOSED;

}
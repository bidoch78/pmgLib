#pragma once

#include <functional>
#include <string>
#include <cstdint>
#include <esp_http_server.h>
#include <map>

namespace pmg::core {

enum class webSocketState : uint8_t {
    CONNECTING = 0,
    OPEN = 1,
    CLOSING = 2,
    CLOSED = 3
};

enum class webSocketMessageStringType : char {
    STRING = 'S',
    JSON = 'J',
    PINGPONG = '@'
};

class webSocket {

    public:

        using MessageCallback   = std::function<void(int, const std::string&)>;
        using CloseCallback     = std::function<void(int, int32_t, const std::string&)>;
        using PongCallBack      = std::function<void(int)>;

        private:

            httpd_handle_t _server = nullptr;
            int _socketfd = -1;
            webSocketState _state = webSocketState::CLOSED;

            uint32_t _listenerId = 0;

            std::map<uint32_t, MessageCallback> _messageCallbacks;
            std::map<uint32_t, CloseCallback> _closeCallbacks;
            std::map<uint32_t, PongCallBack> _pongCallbacks;
            
        public:

            webSocket() = default;
            webSocket(httpd_handle_t server, int socketfd);
            
            int32_t getSocketFd() const;
            bool isConnected() const;
            webSocketState getReadyState() const;

            void attach(httpd_handle_t server, int socketfd);

            esp_err_t send(const uint8_t* data, size_t length);
            esp_err_t send(const webSocketMessageStringType msgType, const std::string& message);
            esp_err_t ping();

            esp_err_t close(uint16_t code = 1000, const std::string& reason = "");

            uint32_t onMessage(MessageCallback callback);
            uint32_t onClose(CloseCallback callback);
            uint32_t onPong(PongCallBack callback);

            void removeMessageListener(uint32_t id);
            void removeCloseListener(uint32_t id);
            void removePongListener(uint32_t id);

            void dispatchMessage(const std::string& message);
            void dispatchClose(int32_t code, const std::string& reason);
            void dispatchPong();

    };

}
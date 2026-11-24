#pragma once

#include <functional>
#include <Arduino.h>
#include <esp_http_server.h>
#include <map>

namespace pmg::core {

enum class webSocketState : uint8_t {
    CONNECTING,
    OPEN,
    CLOSING,
    CLOSED
};

class webSocket {

    public:

        using MessageCallback = std::function<void(int, const String&)>;
        using CloseCallback   = std::function<void(int, int32_t, const String&)>;

        private:

            httpd_handle_t _server = nullptr;
            int _socketfd = -1;
            webSocketState _state = webSocketState::CLOSED;

            uint32_t _listenerId = 0;

            std::map<uint32_t, MessageCallback> _messageCallbacks;
            std::map<uint32_t, CloseCallback> _closeCallbacks;
            
        public:

            webSocket() = default;
            webSocket(httpd_handle_t server, int socketfd);
            
            int32_t getSocketFd() const;
            bool isConnected() const;
            webSocketState getReadyState() const;

            void attach(httpd_handle_t server, int socketfd);

            esp_err_t send(const uint8_t* data, size_t length);
            esp_err_t send(const String& message);
            esp_err_t close(uint16_t code = 1000, const String& reason = "");

            uint32_t onMessage(MessageCallback callback);
            uint32_t onClose(CloseCallback callback);

            void removeMessageListener(uint32_t id);
            void removeCloseListener(uint32_t id);

            void dispatchMessage(const String& message);
            void dispatchClose(int32_t code, const String& reason);

    };

}
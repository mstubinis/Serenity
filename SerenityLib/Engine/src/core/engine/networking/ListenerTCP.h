#pragma once
#ifndef ENGINE_NETWORKING_LISTENER_TCP_H
#define ENGINE_NETWORKING_LISTENER_TCP_H

namespace Engine::Networking {
    class SocketTCP;
}
namespace Engine::priv {
    class SocketManager;
}

#include <core/engine/networking/ISocket.h>

namespace Engine::Networking {
    class ListenerTCP : public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        private:
            sf::TcpListener  m_Listener;
            std::string      m_IP = "";
            uint16_t         m_Port        = 0;


            ListenerTCP() = delete;

            void update(const float dt) override;
        public:
            ListenerTCP(const uint16_t port, const std::string& ip = "");
            ~ListenerTCP();

            void close();
            void setBlocking(bool blocking) override { m_Listener.setBlocking(blocking); }

            bool isListening() const { return (localPort() != 0); }
            bool isBlocking() const override { return m_Listener.isBlocking(); }
            uint16_t localPort() const override { return m_Listener.getLocalPort(); }

            SocketStatus::Status accept(SocketTCP& tcpSocket);
            SocketStatus::Status accept(sf::TcpSocket& sfmlTcpSocket);

            SocketStatus::Status listen();
    };
};
#endif
#pragma once
#ifndef ENGINE_NETWORKING_LISTENER_TCP_H
#define ENGINE_NETWORKING_LISTENER_TCP_H

namespace Engine::Networking {
    class SocketTCP;
}
namespace Engine::priv {
    class SocketManager;
}

#include <core/engine/networking/SocketInterface.h>

namespace Engine::Networking {
    class ListenerTCP : public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        private:
            sf::TcpListener  m_Listener;
            unsigned short   m_Port        = 0;
            std::string      m_IP          = "";

            ListenerTCP() = delete;

            void update(const float dt) override;
        public:
            ListenerTCP(const unsigned short port, const std::string& ip = "");
            ~ListenerTCP();

            void               close();
            void               setBlocking(bool blocking) override;

            bool               isListening() const;
            bool               isBlocking() const override;
            unsigned short     localPort() const override;

            SocketStatus::Status accept(SocketTCP& tcpSocket);
            SocketStatus::Status accept(sf::TcpSocket& sfmlTcpSocket);

            SocketStatus::Status listen();
    };
};
#endif
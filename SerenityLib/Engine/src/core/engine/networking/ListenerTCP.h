#pragma once
#ifndef ENGINE_NETWORKING_LISTENER_TCP_H
#define ENGINE_NETWORKING_LISTENER_TCP_H

#include <core/engine/networking/SocketInterface.h>
#include <string>

namespace Engine::priv {
    class SocketManager;
}
namespace Engine::Networking {
    class SocketTCP;
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

            void                     close();
            void                     setBlocking(const bool blocking) override;

            const bool               isListening() const;
            const bool               isBlocking() const override;
            sf::TcpListener&         socket() override;
            const unsigned short     localPort() const override;

            const sf::Socket::Status accept(SocketTCP& tcpSocket);
            const sf::Socket::Status accept(sf::TcpSocket& sfmlTcpSocket);

            const sf::Socket::Status listen();
    };
};
#endif
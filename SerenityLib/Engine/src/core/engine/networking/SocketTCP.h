#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H

#include <core/engine/networking/SocketInterface.h>
#include <string>
#include <queue>

namespace Engine::priv {
    class SocketManager;
}
namespace Engine::Networking {
    class SocketTCP: public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        private:
            sf::TcpSocket             m_Socket;
            std::string               m_IP       = "";
            unsigned short            m_Port     = 0;
            std::queue<sf::Packet>    m_PartialPackets;

            const sf::Socket::Status internal_send_packet(sf::Packet&);

            void update(const float dt) override;
        public: 
            SocketTCP();
            SocketTCP(const unsigned short port, const std::string& ip = ""); //client side socket
            ~SocketTCP();

            void                       disconnect();
            const bool                 isConnected() const;

            void                       setBlocking(const bool blocking) override;
            const bool                 isBlocking() const override;
            sf::TcpSocket&             socket() override;
            const unsigned short       localPort() const override;

            const std::string          ip() const;
            const unsigned short       remotePort() const;
    
            const sf::Socket::Status   connect(const unsigned short timeout = 0);

            const sf::Socket::Status   send(sf::Packet& packet);
            const sf::Socket::Status   send(const void* data, size_t size);
            const sf::Socket::Status   send(const void* data, size_t size, size_t sent);

            const sf::Socket::Status   receive(sf::Packet& packet);
            const sf::Socket::Status   receive(void* data, size_t size, size_t received);
    };
};
#endif
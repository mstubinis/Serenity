#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H

#include <core/engine/networking/SocketInterface.h>
#include <string>

namespace Engine::Networking {
    class SocketTCP: public ISocket, public Engine::NonCopyable {
        private:
            sf::TcpSocket   m_Socket;
            std::string     m_IP       = "";
            unsigned short  m_Port     = 0;

        public: 
            SocketTCP() = default;
            SocketTCP(const unsigned short port, const std::string& ip = ""); //client side socket
            ~SocketTCP();

            void                       disconnect();

            void                       setBlocking(const bool blocking);
            const bool                 isBlocking();
            const sf::TcpSocket&       socket();
            const unsigned short       localPort();

            const std::string          ip();
            const unsigned short       remotePort();
    
            const sf::Socket::Status   connect(const unsigned short& timeout = 0);

            const sf::Socket::Status   send(sf::Packet& packet);
            const sf::Socket::Status   send(const void* data, size_t size);
            const sf::Socket::Status   send(const void* data, size_t size, size_t& sent);

            const sf::Socket::Status   receive(sf::Packet& packet);
            const sf::Socket::Status   receive(void* data, size_t size, size_t& received);
    };
};
#endif
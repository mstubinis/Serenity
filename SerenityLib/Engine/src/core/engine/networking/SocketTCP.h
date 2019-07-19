#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H


#include <core/engine/networking/SocketInterface.h>

typedef std::uint32_t   uint;

namespace Engine {
    namespace Networking {
        class SocketTCP: public ISocket {
            private:
                sf::TcpSocket*  m_Socket;
                std::string     m_IP;
                ushort          m_Port;
            public: 
                SocketTCP(const ushort port, const std::string& ip = ""); //client side socket
                SocketTCP(sf::TcpSocket*); //server side client socket
                ~SocketTCP();

                void                       disconnect();

                void                       setBlocking(const bool blocking);
                const bool                 isBlocking();
                const sf::TcpSocket&       socket();
                const ushort               localPort();

                const std::string          ip();
                const ushort               remotePort();
    
                const sf::Socket::Status   connect(const ushort& timeout = 0);

                const sf::Socket::Status   send(sf::Packet& packet);
                const sf::Socket::Status   send(const void* data, size_t size);
                const sf::Socket::Status   send(const void* data, size_t size, size_t& sent);

                const sf::Socket::Status   receive(sf::Packet& packet);
                const sf::Socket::Status   receive(void* data, size_t size, size_t& received);
        };
    };
};



#endif
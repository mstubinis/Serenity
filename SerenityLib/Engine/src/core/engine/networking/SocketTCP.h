#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H


#include <core/engine/networking/SocketInterface.h>

typedef std::uint32_t   uint;

namespace Engine {
    namespace Networking {
        class SocketTCP: public ISocket {
            private:
                sf::TcpSocket  m_Socket;
                std::string    m_IP;
                ushort         m_Port;
                ushort         m_Timeout;
            public: 
                SocketTCP(const uint port, const std::string& ip = "", uint timeout = 0);
                ~SocketTCP();

                void setBlocking(const bool);
                const bool isBlocking();
                const sf::TcpSocket& socket();
                const ushort localPort();

                const std::string ip();
                const ushort remotePort();
    
                void connect();
                void disconnect();
                void send(sf::Packet& packet);
                void send(const void* data, size_t size);
                void send(const void* data, size_t size, size_t& sent);
                void receive(sf::Packet& packet);
                void receive(void* data, size_t size, size_t& received);
        };
    };
};



#endif
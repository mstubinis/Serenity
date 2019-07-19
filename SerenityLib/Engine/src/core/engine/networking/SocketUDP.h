#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_UDP_H
#define ENGINE_NETWORKING_SOCKET_UDP_H

#include <core/engine/networking/SocketInterface.h>

typedef std::uint32_t   uint;

namespace Engine {
    namespace Networking {
        class SocketUDP : public ISocket {
            private:
                sf::UdpSocket m_Socket;
                ushort        m_Port;
            public:
                SocketUDP(const uint port);
                ~SocketUDP();

                void                       setBlocking(const bool blocking);
                const bool                 isBlocking();
                const sf::UdpSocket&       socket();
                const ushort               localPort();

                const sf::Socket::Status   bind(const std::string& _ip = "");
                void                       unbind();

                const sf::Socket::Status   send(sf::Packet& packet, const std::string& _ip = "");
                const sf::Socket::Status   send(const void* data, size_t size, const std::string& _ip = "");
                const sf::Socket::Status   receive(sf::Packet& packet, const std::string& _ip = "");
                const sf::Socket::Status   receive(void* data, size_t size, size_t& received, const std::string& _ip = "");
        };
    };
};

#endif
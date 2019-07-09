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

                void setBlocking(const bool);
                const bool isBlocking();
                const sf::UdpSocket& socket();
                const ushort localPort();

                void bind(const std::string& _ip = "");
                void unbind();
                void send(sf::Packet& packet, const std::string& _ip = "");
                void send(const void* data, size_t size, const std::string& _ip = "");
                void receive(sf::Packet& packet, const std::string& _ip = "");
                void receive(void* data, size_t size, size_t& received, const std::string& _ip = "");
        };
    };
};

#endif
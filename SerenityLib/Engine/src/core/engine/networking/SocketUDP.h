#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_UDP_H
#define ENGINE_NETWORKING_SOCKET_UDP_H

#include <core/engine/networking/SocketInterface.h>
#include <string>

namespace Engine::Networking {
    class SocketUDP : public ISocket, public Engine::NonCopyable {
        private:
            sf::UdpSocket   m_Socket;
            unsigned short  m_Port      = 0;
            sf::IpAddress   m_IP        = "";

            SocketUDP() = delete;
        public:
            SocketUDP(const unsigned short port, const std::string& ip = "");
            ~SocketUDP();

            void                       setBlocking(const bool blocking);
            const bool                 isBlocking();
            const sf::UdpSocket&       socket();
            const unsigned short       localPort();

            const sf::Socket::Status   bind(const std::string& ip = "");
            void                       unbind();

            const sf::Socket::Status   send(sf::Packet& packet, const std::string& ip = "");
            const sf::Socket::Status   send(const void* data, size_t size, const std::string& ip = "");
            const sf::Socket::Status   receive(sf::Packet& packet);
            const sf::Socket::Status   receive(void* data, size_t size, size_t received);


            const sf::Socket::Status   send(const unsigned short port, sf::Packet& packet, const std::string& ip = "");
            const sf::Socket::Status   send(const unsigned short port, const void* data, size_t size, const std::string& ip = "");
    };
};

#endif
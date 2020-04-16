#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_UDP_H
#define ENGINE_NETWORKING_SOCKET_UDP_H

#include <core/engine/networking/SocketInterface.h>
#include <string>
#include <queue>

namespace Engine::priv {
    class SocketManager;
}
namespace Engine::Networking {
    class SocketUDP : public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        private:

            struct UDPPacketInfo final {
                sf::Packet      packet;
                unsigned short  port;
                sf::IpAddress   ip;
            };

            sf::UdpSocket               m_Socket;
            unsigned short              m_Port      = 0;
            std::string                 m_IP        = "";
            std::queue<UDPPacketInfo>   m_PartialPackets;

            SocketUDP() = delete;

            sf::IpAddress internal_ip(const std::string& ip) const;

            const sf::Socket::Status internal_send_packet(UDPPacketInfo&);

            void update(const float dt) override;
        public:
            SocketUDP(const unsigned short port, const std::string& ip = "");
            ~SocketUDP();

            void                       setBlocking(const bool blocking) override;
            const bool                 isBlocking() const override;
            const bool                 isBound() const;
            sf::UdpSocket&             socket() override;
            const unsigned short       localPort() const override;

            const sf::Socket::Status   bind(const std::string& ip = "");
            void                       unbind();
            void                       changePort(const unsigned short newPort);
            
            const sf::Socket::Status   send(sf::Packet& packet, const std::string& ip = "");
            const sf::Socket::Status   send(const void* data, size_t size, const std::string& ip = "");
            const sf::Socket::Status   receive(sf::Packet& packet);
            const sf::Socket::Status   receive(void* data, size_t size, size_t received);


            const sf::Socket::Status   send(const unsigned short port, sf::Packet& packet, const std::string& ip = "");
            const sf::Socket::Status   send(const unsigned short port, const void* data, size_t size, const std::string& ip = "");
    };
};

#endif
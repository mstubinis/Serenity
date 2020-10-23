#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_UDP_H
#define ENGINE_NETWORKING_SOCKET_UDP_H

#include <core/engine/networking/ISocket.h>
#include <core/engine/networking/Packet.h>

namespace Engine::priv {
    class SocketManager;
};
namespace Engine::Networking {
    class SocketUDP : public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        public:
            struct UDPPacketInfo final {
                std::unique_ptr<sf::Packet> sfmlPacket;
                sf::IpAddress               ip           = sf::IpAddress::LocalHost;
                uint16_t                    port         = 0;
   
                UDPPacketInfo(uint16_t inPort, sf::IpAddress&& inAddress, sf::Packet* inSFMLPacket)
                    : port{ inPort }
                    , ip{ std::move(inAddress) }
                    , sfmlPacket{ std::unique_ptr<sf::Packet>(inSFMLPacket) }
                {}
                UDPPacketInfo(const UDPPacketInfo& other)            = delete;
                UDPPacketInfo& operator=(const UDPPacketInfo& other) = delete;
            };
        private:
            sf::UdpSocket               m_SocketUDP;
            std::queue<UDPPacketInfo>   m_PartialPackets;
            std::string                 m_IP = "";
            uint16_t                    m_Port = 0;

            SocketUDP() = delete;

            sf::IpAddress internal_get_ip(const std::string& ipString) const;

            SocketStatus::Status internal_send_partial_packets_loop();
            SocketStatus::Status internal_send_packet(UDPPacketInfo& packetInfoUDP);

            void update(const float dt) override;
        public:
            SocketUDP(uint16_t port, const std::string& ip = "");
            ~SocketUDP();

            void clearPartialPackets();

            void                   setBlocking(bool blocking) override;
            bool                   isBlocking() const override;
            bool                   isBound() const;
            uint16_t               localPort() const override;

            SocketStatus::Status   bind(const std::string& ip = "");
            void                   unbind();
            void                   changePort(uint16_t newPort);
            
            SocketStatus::Status   send(Engine::Networking::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(sf::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(const void* data, size_t size, const std::string& ip = "");
            SocketStatus::Status   receive(sf::Packet& packet, sf::IpAddress& ip, uint16_t& port);
            SocketStatus::Status   receive(Engine::Networking::Packet& packet, sf::IpAddress& ip, uint16_t& port);
            SocketStatus::Status   receive(void* data, size_t size, size_t& received, sf::IpAddress& ip, uint16_t& port);

            SocketStatus::Status   send(uint16_t port, Engine::Networking::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(uint16_t port, sf::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(uint16_t port, const void* data, size_t size, const std::string& ip = "");
    };
};

#endif
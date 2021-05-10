#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_UDP_H
#define ENGINE_NETWORKING_SOCKET_UDP_H

namespace Engine::priv {
    class  SocketManager;
    class  EditorWindowScene;
};

#include <serenity/networking/ISocket.h>
#include <serenity/networking/Packet.h>
#include <serenity/system/TypeDefs.h>
#include <queue>

namespace Engine::Networking {
    class SocketUDP : public ISocket {
        friend class Engine::priv::SocketManager;
        friend class Engine::priv::EditorWindowScene;
        public:
            struct UDPPacketInfo final {
                Engine::Networking::Packet  packet;
                sf::IpAddress               ip           = sf::IpAddress::LocalHost;
                uint16_t                    port         = 0;
   
                UDPPacketInfo(uint16_t inPort, sf::IpAddress&& inAddress, Engine::Networking::Packet& packet_)
                    : port{ inPort }
                    , ip{ std::move(inAddress) }
                    , packet{ packet_ }
                {}
                UDPPacketInfo(const UDPPacketInfo&)            = delete;
                UDPPacketInfo& operator=(const UDPPacketInfo&) = delete;
            };
        private:
            sf::UdpSocket               m_SocketUDP;
            std::deque<UDPPacketInfo>   m_PartialPackets;
            sf::IpAddress               m_IP                = sf::IpAddress::LocalHost;
            uint16_t                    m_Port              = 0;

            SocketUDP() = delete;

            SocketStatus::Status internal_send_partial_packets_loop();
            SocketStatus::Status internal_send_packet(UDPPacketInfo& packetInfoUDP);

            void update(const float dt) override;
        public:
            SocketUDP(const SocketUDP&) = delete;
            SocketUDP& operator=(const SocketUDP&) = delete;
            SocketUDP(uint16_t port, sf::IpAddress = sf::IpAddress::LocalHost);
            ~SocketUDP();

            void clearPartialPackets();
            [[nodiscard]] uint32_t getNumPartialPackets() const noexcept;

            void                   setBlocking(bool blocking) override;
            [[nodiscard]] bool                   isBlocking() const override;
            [[nodiscard]] bool                   isBound() const;
            [[nodiscard]] uint16_t               localPort() const override;

            SocketStatus::Status   bind(sf::IpAddress ip = sf::IpAddress::Any);
            void                   unbind();
            void                   changePort(uint16_t newPort);
           
            SocketStatus::Status   receive(Engine::Networking::Packet& packet, sf::IpAddress& ip, uint16_t& port);
            SocketStatus::Status   receive(void* data, size_t size, size_t& received, sf::IpAddress& ip, uint16_t& port);

            SocketStatus::Status   send(Engine::Networking::Packet& packet, sf::IpAddress ip);
            SocketStatus::Status   send(const void* data, size_t size, sf::IpAddress ip);
            SocketStatus::Status   send(uint16_t port, Engine::Networking::Packet& packet, sf::IpAddress ip);
            SocketStatus::Status   send(uint16_t port, const void* data, size_t size, sf::IpAddress ip);

            inline SocketStatus::Status send(Engine::Networking::Packet& packet) noexcept { return send(m_Port, packet, m_IP); }
            inline SocketStatus::Status send(const void* data, size_t size) noexcept { return send(data, size, m_IP); }
            inline SocketStatus::Status send(uint16_t port, Engine::Networking::Packet& packet) noexcept { return send(port, packet, m_IP); }
            inline SocketStatus::Status send(uint16_t port, const void* data, size_t size) noexcept { return send(port, data, size, m_IP); }
    };
};

#endif
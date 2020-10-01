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
                unsigned short              port         = 0;
                sf::IpAddress               ip           = sf::IpAddress::LocalHost;

                UDPPacketInfo(sf::Packet* inSFMLPacket, unsigned short inPort, sf::IpAddress&& inAddress);
                UDPPacketInfo(const UDPPacketInfo& other)            = delete;
                UDPPacketInfo& operator=(const UDPPacketInfo& other) = delete;
            };
        private:
            sf::UdpSocket               m_SocketUDP;
            unsigned short              m_Port      = 0;
            std::string                 m_IP        = "";
            std::queue<UDPPacketInfo>   m_PartialPackets;

            SocketUDP() = delete;

            sf::IpAddress internal_get_ip(const std::string& ipString) const;

            SocketStatus::Status internal_send_partial_packets_loop();
            SocketStatus::Status internal_send_packet(UDPPacketInfo& packetInfoUDP);

            void update(const float dt) override;
        public:
            SocketUDP(unsigned short port, const std::string& ip = "");
            ~SocketUDP();

            void clearPartialPackets();

            void                   setBlocking(bool blocking) override;
            bool                   isBlocking() const override;
            bool                   isBound() const;
            unsigned short         localPort() const override;

            SocketStatus::Status   bind(const std::string& ip = "");
            void                   unbind();
            void                   changePort(unsigned short newPort);
            
            SocketStatus::Status   send(Engine::Networking::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(sf::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(const void* data, size_t size, const std::string& ip = "");
            SocketStatus::Status   receive(sf::Packet& packet, sf::IpAddress& ip, unsigned short& port);
            SocketStatus::Status   receive(Engine::Networking::Packet& packet, sf::IpAddress& ip, unsigned short& port);
            SocketStatus::Status   receive(void* data, size_t size, size_t& received, sf::IpAddress& ip, unsigned short& port);

            SocketStatus::Status   send(unsigned short port, Engine::Networking::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(unsigned short port, sf::Packet& packet, const std::string& ip = "");
            SocketStatus::Status   send(unsigned short port, const void* data, size_t size, const std::string& ip = "");
    };
};

#endif
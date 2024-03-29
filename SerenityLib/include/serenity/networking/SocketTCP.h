#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H

#include <serenity/networking/ISocket.h>
#include <serenity/networking/Packet.h>
#include <serenity/system/TypeDefs.h>
#include <queue>

namespace Engine::priv {
    class SocketManager;
};
namespace Engine::Networking {
    class ListenerTCP;
};
namespace Engine::Networking {
    class SocketTCP: public ISocket {
        friend class Engine::priv::SocketManager;
        friend class Engine::Networking::ListenerTCP;
        private:
            sf::TcpSocket                             m_SocketTCP;
            std::queue<Engine::Networking::Packet>    m_PartialPackets;
            sf::IpAddress                             m_IP;
            uint16_t                                  m_Port     = 0;

            SocketStatus::Status internal_send_partial_packets_loop();
            SocketStatus::Status internal_send_packet(Engine::Networking::Packet&);

            void update(const float dt) override;
        public: 
            SocketTCP();
            SocketTCP(uint16_t port, sf::IpAddress);
            SocketTCP(const SocketTCP&)                 = delete;
            SocketTCP& operator=(const SocketTCP&)      = delete;
            virtual ~SocketTCP();

            void disconnect();
            void setBlocking(bool blocking) override { m_SocketTCP.setBlocking(blocking); }

            [[nodiscard]] bool isBlocking() const override { return m_SocketTCP.isBlocking(); }
            [[nodiscard]] inline bool isConnected() const { return (m_SocketTCP.getLocalPort() != 0); }
            [[nodiscard]] inline sf::IpAddress ip() const { return m_SocketTCP.getRemoteAddress(); }
            [[nodiscard]] inline uint16_t remotePort() const { return m_SocketTCP.getRemotePort(); }
            [[nodiscard]] uint16_t localPort() const override { return m_SocketTCP.getLocalPort(); }

            SocketStatus::Status connect(uint16_t timeout = 0);
            SocketStatus::Status send(Engine::Networking::Packet&);

            //TODO: handle this case automatically
            //DO NOT use this send function if your socket is NON BLOCKING, use the one with the sent parameter instead
            SocketStatus::Status send(const void* data, size_t size);
            SocketStatus::Status send(const void* data, size_t size, size_t& sent);

            SocketStatus::Status receive(Engine::Networking::Packet&);
            SocketStatus::Status receive(void* data, size_t size, size_t& received);
    };
};
#endif
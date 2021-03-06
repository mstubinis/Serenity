#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H

#include <core/engine/networking/ISocket.h>
#include <core/engine/networking/Packet.h>

namespace Engine::priv {
    class SocketManager;
};
namespace Engine::Networking {
    class ListenerTCP;
};
namespace Engine::Networking {
    class SocketTCP: public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        friend class Engine::Networking::ListenerTCP;
        private:
            sf::TcpSocket             m_SocketTCP;
            std::queue<sf::Packet>    m_PartialPackets;
            sf::IpAddress             m_IP;
            uint16_t                  m_Port     = 0;

            SocketStatus::Status internal_send_partial_packets_loop();
            SocketStatus::Status internal_send_packet(sf::Packet& sfPacket);
            SocketStatus::Status internal_send_packet(Engine::Networking::Packet& packet);

            void update(const float dt) override;
        public: 
            SocketTCP();
            SocketTCP(uint16_t port, sf::IpAddress ip);
            ~SocketTCP();

            void                 disconnect();


            bool                 isConnected() const { return (m_SocketTCP.getLocalPort() != 0); }
            sf::IpAddress        ip() const { return m_SocketTCP.getRemoteAddress(); }
            uint16_t             remotePort() const { return m_SocketTCP.getRemotePort(); }
            uint16_t             localPort() const override { return m_SocketTCP.getLocalPort(); }


            void                 setBlocking(bool blocking) override { m_SocketTCP.setBlocking(blocking); }
            bool                 isBlocking() const override { return m_SocketTCP.isBlocking(); }

    
            SocketStatus::Status   connect(uint16_t timeout = 0);
            SocketStatus::Status   send(sf::Packet& packet);

            //TODO: handle this case automatically
            //DO NOT use this send function if your socket is NON BLOCKING, use the one with the sent parameter instead
            SocketStatus::Status   send(const void* data, size_t size);
            SocketStatus::Status   send(const void* data, size_t size, size_t& sent);

            SocketStatus::Status   receive(sf::Packet& packet);
            SocketStatus::Status   receive(void* data, size_t size, size_t& received);
    };
};
#endif
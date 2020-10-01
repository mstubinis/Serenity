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
            std::string               m_IP       = "";
            unsigned short            m_Port     = 0;
            std::queue<sf::Packet>    m_PartialPackets;

            SocketStatus::Status internal_send_partial_packets_loop();
            SocketStatus::Status internal_send_packet(sf::Packet& sfPacket);
            SocketStatus::Status internal_send_packet(Engine::Networking::Packet& packet);

            void update(const float dt) override;
        public: 
            SocketTCP();
            SocketTCP(const unsigned short port, const std::string& ip = ""); //client side socket
            ~SocketTCP();

            void                 disconnect();


            bool                 isConnected() const { return (m_SocketTCP.getLocalPort() != 0); }
            std::string          ip() const { return m_SocketTCP.getRemoteAddress().toString(); }
            unsigned short       remotePort() const { return m_SocketTCP.getRemotePort(); }
            unsigned short       localPort() const override { return m_SocketTCP.getLocalPort(); }


            void                 setBlocking(bool blocking) override { m_SocketTCP.setBlocking(blocking); }
            bool                 isBlocking() const override { return m_SocketTCP.isBlocking(); }

    
            SocketStatus::Status   connect(const unsigned short timeout = 0);

            //SocketStatus::Status   send(Engine::Networking::Packet& packet);
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
#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_TCP_H
#define ENGINE_NETWORKING_SOCKET_TCP_H

#include <core/engine/networking/SocketInterface.h>
#include <core/engine/networking/Packet.h>
#include <string>
#include <queue>

namespace Engine::priv {
    class SocketManager;
}
namespace Engine::Networking {
    class SocketTCP: public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        private:
            sf::TcpSocket             m_SocketTCP;
            std::string               m_IP       = "";
            unsigned short            m_Port     = 0;
            std::queue<sf::Packet>    m_PartialPackets;

            sf::Socket::Status internal_send_partial_packets_loop();
            sf::Socket::Status internal_send_packet(sf::Packet& sfPacket);
            sf::Socket::Status internal_send_packet(Engine::Networking::Packet& packet);

            void update(const float dt) override;
        public: 
            SocketTCP();
            SocketTCP(const unsigned short port, const std::string& ip = ""); //client side socket
            ~SocketTCP();

            void                 disconnect();
            bool                 isConnected() const;

            void                 setBlocking(bool blocking) override;
            bool                 isBlocking() const override;
            sf::TcpSocket&       socket() override;
            unsigned short       localPort() const override;

            std::string          ip() const;
            unsigned short       remotePort() const;
    
            sf::Socket::Status   connect(const unsigned short timeout = 0);

            sf::Socket::Status   send(Engine::Networking::Packet& packet);
            sf::Socket::Status   send(sf::Packet& packet);

            //TODO: handle this case automatically
            //DO NOT use this send function if your socket is NON BLOCKING, use the one with the sent parameter instead
            sf::Socket::Status   send(const void* data, size_t size);


            sf::Socket::Status   send(const void* data, size_t size, size_t& sent);

            sf::Socket::Status   receive(sf::Packet& packet);
            sf::Socket::Status   receive(void* data, size_t size, size_t& received);
    };
};
#endif
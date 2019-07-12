#pragma once
#ifndef ENGINE_NETWORKING_LISTENER_TCP_H
#define ENGINE_NETWORKING_LISTENER_TCP_H

#include <core/engine/networking/SocketInterface.h>

typedef std::uint32_t   uint;


namespace Engine {
    namespace Networking {
        class SocketTCP;
        class ListenerTCP : public ISocket {
            private:
                sf::TcpListener  m_Listener;
                std::string      m_IP;
                ushort           m_Port;
            public:
                ListenerTCP(const uint port, const std::string& ip = "");
                ~ListenerTCP();

                void setBlocking(bool);
                const bool isBlocking();
                const sf::TcpListener& socket();
                const ushort localPort();

                const std::string& ip();
                void close();
                const sf::Socket::Status accept(SocketTCP&);
                const sf::Socket::Status accept(sf::TcpSocket&);

                const sf::Socket::Status listen();
        };
    };
};

#endif
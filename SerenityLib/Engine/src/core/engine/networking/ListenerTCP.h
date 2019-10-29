#pragma once
#ifndef ENGINE_NETWORKING_LISTENER_TCP_H
#define ENGINE_NETWORKING_LISTENER_TCP_H

#include <core/engine/networking/SocketInterface.h>
#include <core/engine/utils/Utils.h>

namespace Engine {
    namespace Networking {
        class SocketTCP;
        class ListenerTCP : public ISocket {
            private:
                sf::TcpListener  m_Listener;
                ushort           m_Port;
                std::string      m_Ip;
            public:
                ListenerTCP(const uint port, const std::string& ip = "");
                ~ListenerTCP();

                void                     close();
                void                     setBlocking(const bool blocking);

                const bool               isBlocking();
                const sf::TcpListener&   socket();
                const ushort             localPort();

                const sf::Socket::Status accept(SocketTCP& tcpSocket);
                const sf::Socket::Status accept(sf::TcpSocket& sfmlTcpSocket);

                const sf::Socket::Status listen();
        };
    };
};
#endif
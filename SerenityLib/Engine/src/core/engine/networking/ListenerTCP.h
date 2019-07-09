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
                sf::TcpListener m_Listener;
                std::string     m_IP;
                ushort          m_Port;
            public:
                ListenerTCP(const uint port, const std::string& ip = "");
                ~ListenerTCP();

                const sf::TcpListener& socket();
                const std::string ip();
                const ushort localPort();
                void setBlocking(bool);
                const bool isBlocking();
                void close();
                void accept(SocketTCP&);
                void accept(SocketTCP*);
                void listen();
        };
    };
};

#endif
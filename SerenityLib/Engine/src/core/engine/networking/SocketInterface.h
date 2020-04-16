#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_INTERFACE_H
#define ENGINE_NETWORKING_SOCKET_INTERFACE_H

#include <SFML/Network.hpp>

struct SocketType final {
    enum Type : unsigned char {
        Unknown,
        TCP,
        UDP,
        TCPListener,
    };
};

namespace Engine::Networking {
    class ISocket {
        private:
            virtual void                   update(const float dt) = 0;
        public:
            virtual void                   setBlocking(const bool blocking) = 0;
            virtual const bool             isBlocking() const = 0;
            virtual sf::Socket&            socket() = 0;
            virtual const unsigned short   localPort() const = 0;
    };
};

#endif
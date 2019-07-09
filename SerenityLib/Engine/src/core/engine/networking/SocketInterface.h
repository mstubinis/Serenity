#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_INTERFACE_H
#define ENGINE_NETWORKING_SOCKET_INTERFACE_H

#include <SFML/Network.hpp>

typedef unsigned short  ushort;

namespace Engine {
    namespace Networking {
        class ISocket {
            public:
                virtual void setBlocking(const bool) = 0;
                virtual const bool isBlocking() = 0;
                virtual const sf::Socket& socket() = 0;
                virtual const ushort localPort() = 0;
        };
    };
};

#endif
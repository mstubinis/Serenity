#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_INTERFACE_H
#define ENGINE_NETWORKING_SOCKET_INTERFACE_H

#include <SFML/Network.hpp>

namespace Engine::Networking {
    class ISocket {
        public:
            virtual void                   setBlocking(const bool blocking) = 0;
            virtual const bool             isBlocking() = 0;
            virtual const sf::Socket&      socket() = 0;
            virtual const unsigned short   localPort() = 0;
    };
};

#endif
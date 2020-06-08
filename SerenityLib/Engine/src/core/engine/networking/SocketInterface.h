#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_INTERFACE_H
#define ENGINE_NETWORKING_SOCKET_INTERFACE_H

#include <SFML/Network.hpp>

struct SocketType final { enum Type : unsigned char {
    Unknown,
    TCP,
    UDP,
    TCPListener,
};};

struct SocketStatus final {
    enum Status : unsigned char {
        Done         = sf::Socket::Status::Done,         //0
        NotReady     = sf::Socket::Status::NotReady,     //1
        Partial      = sf::Socket::Status::Partial,      //2
        Disconnected = sf::Socket::Status::Disconnected, //3
        Error        = sf::Socket::Status::Error,        //4

    };
    static SocketStatus::Status map_status(sf::Socket::Status sfmlStatus);
};

namespace Engine::Networking {
    class ISocket {
        private:
            virtual void             update(const float dt) = 0;
        public:
            virtual void             setBlocking(bool blocking) = 0;
            virtual bool             isBlocking() const = 0;
            //virtual sf::Socket&      getSFMLSocket() = 0;
            virtual unsigned short   localPort() const = 0;
    };
};

#endif
#pragma once
#ifndef ENGINE_NETWORKING_I_SOCKET_H
#define ENGINE_NETWORKING_I_SOCKET_H

enum class SocketType : uint8_t {
    Unknown,
    TCP,
    UDP,
    TCPListener,
};

#include <SFML/Network.hpp>

struct SocketStatus final {
    enum Status : uint8_t {
        Done         = sf::Socket::Status::Done,         //0
        NotReady     = sf::Socket::Status::NotReady,     //1
        Partial      = sf::Socket::Status::Partial,      //2
        Disconnected = sf::Socket::Status::Disconnected, //3
        Error        = sf::Socket::Status::Error,        //4

    };
    static inline SocketStatus::Status map_status(sf::Socket::Status sfmlStatus) noexcept { return (SocketStatus::Status)sfmlStatus; }
};
namespace Engine::Networking {
    class ISocket {
        private:
            virtual void       update(const float dt) = 0;
        public:
            virtual void       setBlocking(bool blocking) = 0;
            virtual bool       isBlocking() const = 0;
            virtual uint16_t   localPort() const = 0;
    };
};
#endif
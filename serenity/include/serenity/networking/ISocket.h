#pragma once
#ifndef ENGINE_NETWORKING_I_SOCKET_H
#define ENGINE_NETWORKING_I_SOCKET_H

#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

class SocketType {
    public:
        enum Type : uint8_t {
            Unknown = 0,
            TCP,
            UDP,
            TCPListener,
        };
        BUILD_ENUM_CLASS_MEMBERS(SocketType, Type)
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
    [[nodiscard]] static inline SocketStatus::Status map_status(sf::Socket::Status sfmlStatus) noexcept { return (SocketStatus::Status)sfmlStatus; }
};
namespace Engine::Networking {
    class ISocket {
        private:
            virtual void                     update(const float dt) = 0;
        public:
            virtual void                     setBlocking(bool blocking) = 0;
            [[nodiscard]] virtual bool       isBlocking() const = 0;
            [[nodiscard]] virtual uint16_t   localPort() const = 0;
    };
};
#endif
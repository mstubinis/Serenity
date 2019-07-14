#pragma once
#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <core/engine/networking/SocketTCP.h>

struct Packet;
class Client;
namespace Engine {
    namespace epriv {
        struct ClientInternalPublicInterface {
            static void update(Client*);
        };
    };
};
class Client{
    friend struct Engine::epriv::ClientInternalPublicInterface;
    private:
        Engine::Networking::SocketTCP* m_client;
    public:
        Client(sf::TcpSocket*);
        Client(const ushort& port, const std::string& ipAddress);
        ~Client();

        void changeConnectionDestination(const ushort& port, const std::string& ipAddress);

        void connect();
        void connect(const ushort& timeout);
        void disconnect();
        void onReceive();

        const bool connected() const;
        const sf::Socket::Status send(Packet& packet);
        const sf::Socket::Status send(sf::Packet& packet);
        const sf::Socket::Status send(const void* data, size_t size);
        const sf::Socket::Status send(const void* data, size_t size, size_t& sent);

        const sf::Socket::Status receive(sf::Packet& packet);
        const sf::Socket::Status receive(void* data, size_t size, size_t& received);

};
#endif
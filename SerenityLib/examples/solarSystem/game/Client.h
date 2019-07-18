#pragma once
#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <core/engine/networking/SocketTCP.h>

#include <thread>
#include <future>

struct Packet;
class  Client;
class  Server;
class  HUD;
class  Core;
namespace Engine {
    namespace epriv {
        struct ClientInternalPublicInterface {
            static void update(Client*);
        };
    };
};
class Client{
    friend class  Server;
    friend class  HUD;
    friend class  Core;
    friend struct Engine::epriv::ClientInternalPublicInterface;
    private:
        std::thread*                   m_InitialConnectionThread;
        Engine::Networking::SocketTCP* m_TcpSocket;
        std::string                    m_username;
        Core&                          m_Core;
        bool                           m_Validated;
        bool                           m_IsCurrentlyConnecting;
    public:
        Client(Core&, sf::TcpSocket*);
        Client(Core&, const ushort& port, const std::string& ipAddress);
        ~Client();

        void changeConnectionDestination(const ushort& port, const std::string& ipAddress);

        const sf::Socket::Status connect(const ushort& timeout = 0);
        void disconnect();
        void onReceive();

        const std::string& username() const;
        const sf::Socket::Status send(Packet& packet);
        const sf::Socket::Status send(sf::Packet& packet);
        const sf::Socket::Status send(const void* data, size_t size);
        const sf::Socket::Status send(const void* data, size_t size, size_t& sent);

        const sf::Socket::Status receive(sf::Packet& packet);
        const sf::Socket::Status receive(void* data, size_t size, size_t& received);

};
#endif
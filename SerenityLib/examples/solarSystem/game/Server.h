#pragma once
#ifndef GAME_SERVER_H
#define GAME_SERVER_H


#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>

#include "Client.h"


#include <unordered_map>
//#include <thread>

struct Packet;
class  Server;
namespace Engine {
    namespace epriv {
        struct ServerInternalPublicInterface {
            static void update(Server*);
        };
    };
};

class Server {
    friend struct Engine::epriv::ServerInternalPublicInterface;
    private:
        //std::thread*                            m_thread;
        std::unordered_map<std::string, Client*>  m_clients;
        Engine::Networking::ListenerTCP*          m_listener;
        unsigned int                              m_port;
        bool                                      m_active;
    public:
        Server(const unsigned int& port);
        ~Server();

        const bool startup();
        void shutdown();
        const bool& isActive() const;

        const sf::Socket::Status send_to_client(Client&, Packet& packet);
        const sf::Socket::Status send_to_client(Client&, sf::Packet& packet);
        const sf::Socket::Status send_to_client(Client&, const void* data, size_t size);
        const sf::Socket::Status send_to_client(Client&, const void* data, size_t size, size_t& sent);

        const void send_to_all_but_client(Client&, Packet& packet);
        const void send_to_all_but_client(Client&, sf::Packet& packet);
        const void send_to_all_but_client(Client&, const void* data, size_t size);
        const void send_to_all_but_client(Client&, const void* data, size_t size, size_t& sent);

        const void send_to_all(Packet& packet);
        const void send_to_all(sf::Packet& packet);
        const void send_to_all(const void* data, size_t size);
        const void send_to_all(const void* data, size_t size, size_t& sent);

        void onReceive();
};

#endif
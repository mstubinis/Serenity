#pragma once
#ifndef GAME_SERVER_H
#define GAME_SERVER_H


#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>

#include "Client.h"

#include <unordered_map>
#include <queue>
#include <thread>

struct Packet;
class  Server;
class  Core;

class Server {
    private:
        sf::Mutex                                      m_mutex;
        std::thread*                                   m_thread_for_listener;
        std::thread*                                   m_thread_for_disconnecting_clients;
        std::unordered_map<std::string, std::thread*>  m_threads_for_clients;
        std::unordered_map<std::string, Client*>       m_clients;
        std::queue<std::string>                        m_ClientsToBeDisconnected;
        Engine::Networking::ListenerTCP*               m_listener;
        unsigned int                                   m_port;
        bool                                           m_blocking;
        Core&                                          m_Core;
        std::string                                    m_MapName;
    public:
        Server(Core&, const unsigned int& port, const bool blocking = false, const std::string& ipRestriction = "");
        ~Server();

        const bool startup(const std::string& mapname);
        void shutdown(const bool destructor = false);

        const bool isValidName(const std::string& name) const;

        const sf::Socket::Status send_to_client(Client&, Packet& packet);
        const sf::Socket::Status send_to_client(Client&, sf::Packet& packet);
        const sf::Socket::Status send_to_client(Client&, const void* data, size_t size);
        const sf::Socket::Status send_to_client(Client&, const void* data, size_t size, size_t& sent);

        void send_to_all_but_client(Client&, Packet& packet);
        void send_to_all_but_client(Client&, sf::Packet& packet);
        void send_to_all_but_client(Client&, const void* data, size_t size);
        void send_to_all_but_client(Client&, const void* data, size_t size, size_t& sent);

        void send_to_all(Packet& packet);
        void send_to_all(sf::Packet& packet);
        void send_to_all(const void* data, size_t size);
        void send_to_all(const void* data, size_t size, size_t& sent);

        static void update(Server* thisServer);
        static void updateAcceptNewClients(Server* thisServer);
        static void updateClient(Server* thisServer, Client* client);
        static void updateRemoveDisconnectedClients(Server* thisServer);
};

#endif
#pragma once
#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>

#include <unordered_map>
#include <queue>
#include <thread>
#include <atomic>
#include <future>

struct Packet;
class  Server;
class  Core;

#define SERVER_CLIENT_TIMEOUT 20.0f
#define SERVER_CLIENT_RECOVERY_TIME 60.0f

class ServerClient final {
    friend class Server;
    private:
        std::future<bool>*               m_Thread;
        Engine::Networking::SocketTCP*   m_TcpSocket;
        std::string                      m_username;
        Core&                            m_Core;
        Server&                          m_Server;
        bool                             m_Validated;
        bool                             m_Active;
        double                           m_Timeout;
        double                           m_RecoveryTime;

        void internalInit(const bool blocking);
    public:
        ServerClient(Server&, Core&, sf::TcpSocket*, const bool blocking = false);
        ServerClient(Server&, Core&, const ushort& port, const std::string& ipAddress, const bool blocking = false);
        ~ServerClient();

        void disconnect();
        const bool disconnected() const;

        const std::string& username() const;
        const sf::Socket::Status send(Packet& packet);
        const sf::Socket::Status send(sf::Packet& packet);
        const sf::Socket::Status send(const void* data, size_t size);
        const sf::Socket::Status send(const void* data, size_t size, size_t& sent);

        const sf::Socket::Status receive(sf::Packet& packet);
        const sf::Socket::Status receive(void* data, size_t size, size_t& received);
};


class Server {
    friend class ServerClient;
    private:
        sf::Mutex                                      m_mutex;
        std::unordered_map<std::string, ServerClient*> m_clients;
        std::queue<std::string>                        m_ClientsToBeDisconnected;
        Engine::Networking::ListenerTCP*               m_listener;
        unsigned int                                   m_port;
        bool                                           m_blocking;
        std::atomic<unsigned int>                      m_Active;
        Core&                                          m_Core;
        std::string                                    m_MapName;
        double                                         m_DeepspaceAnchorTimer;

        void updateClientsGameLoop(const double& dt);

    public:
        Server(Core& core, const unsigned int& port, const bool blocking = false, const std::string& ipRestriction = "");
        ~Server();

        const bool startup(const std::string& mapname);
        void shutdown(const bool destructor = false);

        const bool isValidName(const std::string& name) const;

        const sf::Socket::Status send_to_client(ServerClient&, Packet& packet);
        const sf::Socket::Status send_to_client(ServerClient&, sf::Packet& packet);
        const sf::Socket::Status send_to_client(ServerClient&, const void* data, size_t size);
        const sf::Socket::Status send_to_client(ServerClient&, const void* data, size_t size, size_t& sent);

        void send_to_all_but_client(ServerClient&, Packet& packet);
        void send_to_all_but_client(ServerClient&, sf::Packet& packet);
        void send_to_all_but_client(ServerClient&, const void* data, size_t size);
        void send_to_all_but_client(ServerClient&, const void* data, size_t size, size_t& sent);

        void send_to_all(Packet& packet);
        void send_to_all(sf::Packet& packet);
        void send_to_all(const void* data, size_t size);
        void send_to_all(const void* data, size_t size, size_t& sent);

        static void update(Server* thisServer, const double& dt);
        static void updateAcceptNewClients(Server& thisServer);
        static void updateClient(ServerClient& thisClient);
        static void updateRemoveDisconnectedClients(Server& thisServer);
};

#endif
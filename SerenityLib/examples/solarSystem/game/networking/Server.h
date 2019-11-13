#pragma once
#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/networking/SocketUDP.h>

#include <unordered_map>
#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include <mutex>

struct Packet;
class  Map;
class  Server;
class  ServerClient;
class  ServerClientThread;
class  Core;
class  GameplayMode;
struct PacketMessage;
struct PacketCollisionEvent;

#define SERVER_CLIENT_TIMEOUT 20.0f
#define SERVER_CLIENT_RECOVERY_TIME 60.0f

//a simple data structure to coordinate ship respawning
class ShipRespawning final {
    private:
        std::unordered_map<std::string, std::tuple<std::string, std::string, double>> m_Ships; //key = shipkey, value = ship class,  closest spawn anchor, respawn time left
        Server& m_Server;
    public:
        ShipRespawning(Server&);
        ~ShipRespawning();

        void processShip(const std::string& shipMapKey, const std::string& shipClass, const std::string& closest_spawn_anchor);

        void cleanup();
        void update(const double& dt);
};


//a simple data structure to coordinate ship on ship / station / whatever collisions
class CollisionEntries final {
    private:
        std::unordered_map<std::string, double> m_CollisionPairs; //key = ship1.key + "|" + ship2.key, double is time left until another collision entry can be processed
        Server& m_Server;
    public:
        CollisionEntries(Server&);
        ~CollisionEntries();

        void processCollision(const PacketCollisionEvent& packet, Map& map);

        void cleanup();
        void update(const double& dt);

};

class ServerClient final {
    friend class Server;
    friend class ServerClientThread;
    private:
        Engine::Networking::SocketTCP*   m_TcpSocket;
        std::string                      m_Username;
        std::string                      m_MapKey;
        std::string                      m_Hash;
        std::string                      m_IP;
        Core&                            m_Core;
        Server&                          m_Server;
        bool                             m_Validated;
        unsigned int                     m_ID;
        double                           m_Timeout;
        double                           m_RecoveryTime;

        void internalInit(const std::string& hash, const unsigned int& numClients);
    public:
        ServerClient(const std::string& hash, Server&, Core&, sf::TcpSocket*);
        ServerClient(const std::string& hash, Server&, Core&, const unsigned short& port, const std::string& ipAddress);
        ~ServerClient();

        void disconnect();
        const bool disconnected() const;

        const std::string& username() const;
        const std::string& getMapKey() const;
        const sf::Socket::Status send(Packet& packet);
        const sf::Socket::Status send(sf::Packet& packet);
        const sf::Socket::Status send(const void* data, size_t size);
        const sf::Socket::Status send(const void* data, size_t size, size_t& sent);

        const sf::Socket::Status receive(sf::Packet& packet);
        const sf::Socket::Status receive(void* data, size_t size, size_t& received);
};

class ServerClientThread final {
    friend class ServerClient;
    friend class Server;
    private:
        std::unordered_map<std::string, ServerClient*>   m_Clients;
        std::atomic<unsigned int>                        m_Active;
        bool                                             m_Shutdowned;
        std::future<bool>*                               m_Thread;
    public:
        ServerClientThread();
        ~ServerClientThread();
};

class Server {
    friend class ServerClient;
    friend class ServerClientThread;
    private:
        CollisionEntries                               m_CollisionEntries;
        ShipRespawning                                 m_RespawningShips;

        GameplayMode*                                  m_GameplayMode;
        Engine::Networking::SocketUDP*                 m_UdpSocket;
        std::mutex                                     m_Mutex;
        std::vector<ServerClientThread*>               m_Threads;
        std::queue<std::string>                        m_ClientsToBeDisconnected;
        Engine::Networking::ListenerTCP*               m_listener;
        unsigned short                                 m_port;
        std::atomic<unsigned int>                      m_Active;
        Core&                                          m_Core;
        std::string                                    m_MapName;
        double                                         m_DeepspaceAnchorTimer;

        void assign_username_to_client(ServerClient&, const std::string& username);
        void completely_remove_client(ServerClient&);
        void assignRandomTeam(PacketMessage& packet_out, ServerClient& client);
        void updateClientsGameLoop(const double& dt);
        void onReceiveUDP();
    public:
        Server(Core& core, const unsigned int& port, const std::string& ipRestriction = "");
        ~Server();

        const bool startup(const std::string& mapname);
        const bool startupMap(GameplayMode& mode);
        void shutdown(const bool destructor = false);
        const bool shutdownMap();

        const bool isValidName(const std::string& name) const;
        const unsigned int numClients() const;

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

        void send_to_all_but_client_udp(ServerClient& serverClient, Packet& packet);
        void send_to_all_but_client_udp(ServerClient& serverClient, sf::Packet& packet);
        void send_to_all_but_client_udp(ServerClient& serverClient, const void* data, size_t size);

        void send_to_all_udp(Packet& packet);
        void send_to_all_udp(sf::Packet& packet);
        void send_to_all_udp(const void* data, size_t size);
        const sf::Socket::Status receive_udp(sf::Packet& packet);
        const sf::Socket::Status receive_udp(void* data, size_t size, size_t& received);

        ServerClient* getClientByUsername(const std::string& UserName);
        ServerClient* getClientByMapKey(const std::string& MapKey);

        static void update(Server* thisServer, const double& dt);
        static void updateAcceptNewClients(Server& thisServer);
        static void updateClient(ServerClient& thisClient);
        static void updateRemoveDisconnectedClients(Server& thisServer);
};

#endif
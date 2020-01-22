#pragma once
#ifndef GAME_SERVER_H
#define GAME_SERVER_H

struct Packet;
class  ServerClient;
class  ServerClientThread;
class  Core;
struct PacketMessage;

#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/networking/SocketUDP.h>

#include "../auth/AuthenticationLayer.h"

#include "ServerMapSpecificData.h"
#include "../../modes/GameplayMode.h"
#include "../../map/MapEntry.h"
#include "../../database/Database.h"

#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include <mutex>

class ServerHostData final {
    private:
        MapEntryData              m_CurrentMapChoice;
        unsigned short            m_ServerPort;
        GameplayMode              m_GameplayMode;
        unsigned int              m_ExpectedMatchDurationInSeconds;
        double                    m_LobbyTimeInSeconds;
        double                    m_CurrentLobbyTimeInSeconds;
    public:
        const MapEntryData& getMapChoice() const;
        void setMapChoice(const MapEntryData&);

        void setCurrentLobbyTimeInSeconds(const double& seconds);
        const double& getCurrentLobbyTime() const;

        void setLobbyTimeInSeconds(const double& seconds);
        const double& getLobbyTime() const;

        void setMatchDurationInMinutes(const unsigned int& minutes);
        void setMatchDurationInSeconds(const unsigned int& seconds);

        void setServerPort(const unsigned short&);
        const unsigned short& getServerPort() const;

        const GameplayModeType::Mode& getGameplayModeType() const;
        void setGameplayModeType(const GameplayModeType::Mode&);

        const bool addTeam(Team& team);

        void setMaxAmountOfPlayers(const unsigned int&);

        std::unordered_set<std::string>& getAllowedShips();
        void setAllowedShips(const std::vector<std::string>&);
        void setAllowedShips(const std::unordered_set<std::string>&);

        const GameplayMode& getGameplayMode() const;
        const std::string& getGameplayModeString() const;

        ServerHostData();
        ~ServerHostData();
};

class ServerClient final {
    friend class Server;
    friend class AuthenticationLayer;
    friend class AuthenticationLayer::AuthenticationInstance;
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
        ServerClient(const std::string& hash, Server&, Core&, Engine::Networking::SocketTCP&);
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
    friend class AuthenticationLayer;
    friend class AuthenticationLayer::AuthenticationInstance;
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
    friend class ServerMapSpecificData;
    friend class AuthenticationLayer;
    friend class AuthenticationLayer::AuthenticationInstance;
    public: class PersistentInfo final {
        private:
            std::string m_ServerName;
            std::string m_OwnerName;
            unsigned short m_Port;

        public:
            PersistentInfo();

            const std::string& getOwnerName() const;
            const std::string& getServerName() const;
            const unsigned short& getPort() const;

            void setInfo(const std::string& serverName, const unsigned short& port, const std::string& ownerName);
            void clearInfo();

            bool operator==(const bool& rhs) const;
            explicit operator bool() const;
    };

    public:
        static PersistentInfo                          PERSISTENT_INFO;
        static ServerHostData                          SERVER_HOST_DATA;
        static Database                                DATABASE;
    private:
        AuthenticationLayer                            m_AuthenticationLayer;
        ServerMapSpecificData                          m_MapSpecificData;
        Engine::Networking::SocketUDP*                 m_UdpSocket;
        std::mutex                                     m_Mutex;
        std::vector<ServerClientThread*>               m_Threads;
        Engine::Networking::ListenerTCP*               m_TCPListener;
        unsigned short                                 m_Port;
        std::atomic<unsigned int>                      m_Active;
        Core&                                          m_Core;
        ServerClient*                                  m_OwnerClient;

        void assign_username_to_client(ServerClient&, const std::string& username);
        void completely_remove_client(ServerClient&);
        std::string assignRandomTeam(PacketMessage& packet_out, ServerClient& client);
        void updateClientsGameLoop(const double& dt);
        void onReceiveUDP();
    public:
        Server(Core& core, const unsigned int& port, const std::string& ipRestriction = "");
        ~Server();

        const bool startup();
        const bool startupMap(const MapEntryData& map_data);
        void shutdown(const bool destructor = false);
        const bool shutdownMap();

        ServerClientThread* getNextAvailableClientThread();

        AuthenticationLayer& getAuthenticationLayer();

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
};

#endif
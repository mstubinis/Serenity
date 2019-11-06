#pragma once
#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <core/engine/networking/SocketTCP.h>
#include <core/engine/networking/SocketUDP.h>

#include <thread>
#include <future>

struct Packet;
class  Client;
class  Server;
class  Menu;
class  Map;
class  Core;
class  ShipSystemSensors;
class  SensorStatusDisplay;
class  GameplayMode;
class  Team;
class  FireAtWill;
class Client{
    friend class  Server;
    friend class  Menu;
    friend class  Map;
    friend class  Core;
    friend class  ShipSystemSensors;
    friend class  SensorStatusDisplay;
    friend class  FireAtWill;
    private:
        GameplayMode&                         m_GameplayMode;
        Team*                                 m_Team;
        std::future<sf::Socket::Status>*      m_InitialConnectionThread;
        Engine::Networking::SocketTCP*        m_TcpSocket;
        Engine::Networking::SocketUDP*        m_UdpSocket;
        std::string                           m_Username;
        std::string                           m_Mapname;
        std::string                           m_ServerIP;
        Core&                                 m_Core;
        bool                                  m_Validated;
        uint                                  m_ID;
        double                                m_PingTime;
        bool                                  m_IsCurrentlyConnecting;
        ushort                                m_Port;

        void internalInit(const ushort& server_port, const std::string& server_ipAddress);

        void on_receive_anti_cloak_status(Packet*, Map& map);
        void on_receive_server_game_mode(Packet*);
        void on_receive_physics_update(Packet*, Map& map);
        void on_receive_cannon_impact(Packet*, Map& map);
        void on_receive_torpedo_impact(Packet*, Map& map);
        void on_receive_client_left_map(Packet*, Map& map);
        void on_receive_client_fired_cannons(Packet*, Map& map);
        void on_receive_client_fired_beams(Packet*, Map& map);
        void on_receive_client_fired_torpedos(Packet*, Map& map);
        void on_receive_target_changed(Packet*, Map& map);
        void on_receive_create_deep_space_anchor_initial(Packet*, Map& map);
        void on_receive_create_deep_space_anchor(Packet*, Map& map);
        void on_receive_health_update(Packet*, Map& map);
        void on_receive_cloak_update(Packet*, Map& map);
        void on_receive_new_client_entered_map(Packet*);
        void on_receive_new_client_approve_map_entry(Packet*, Menu&);
        void on_receive_map_data(Packet*, Menu&);
        void on_receive_chat_message(Packet*, Menu&);
        void on_receive_client_just_joined_server_lobby(Packet*, Menu&);
        void on_receive_client_just_left_server(Packet*, Menu&);
        void on_receive_connection_accepted_by_server(Packet*, Menu&);
        void on_receive_connection_rejected_by_server(Packet*, Menu&);
        void on_receive_server_shutdown(Packet*, Menu&);
    public:
        Client(GameplayMode& , Team* ,Core&, const ushort& server_port, const std::string& server_ipAddress, const uint& id);
        ~Client();

        void setClientID(const uint id);

        void changeConnectionDestination(const ushort& port, const std::string& ipAddress);

        const sf::Socket::Status connect(const ushort& timeout = 0);
        void disconnect();
        void onReceiveTCP();
        void onReceiveUDP();

        const std::string& username() const;

        const sf::Socket::Status send(Packet& packet);
        const sf::Socket::Status send(sf::Packet& packet);
        const sf::Socket::Status send(const void* data, size_t size);
        const sf::Socket::Status send(const void* data, size_t size, size_t& sent);

        const sf::Socket::Status receive(sf::Packet& packet);
        const sf::Socket::Status receive(void* data, size_t size, size_t& received);

        const sf::Socket::Status send_udp(Packet& packet);
        const sf::Socket::Status send_udp(sf::Packet& packet);
        const sf::Socket::Status send_udp(const void* data, size_t size);

        const sf::Socket::Status receive_udp(sf::Packet& packet);
        const sf::Socket::Status receive_udp(void* data, size_t size, size_t& received);

        static void update(Client*, const double& dt);
};
#endif
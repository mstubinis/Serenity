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
class  Core;
class  ShipSystemSensors;
class  SensorStatusDisplay;
class Client{
    friend class  Server;
    friend class  Menu;
    friend class  Core;
    friend class  ShipSystemSensors;
    friend class  SensorStatusDisplay;
    private:
        std::future<sf::Socket::Status>*      m_InitialConnectionThread;
        Engine::Networking::SocketTCP*        m_TcpSocket;
        Engine::Networking::SocketUDP*        m_UdpSocket;
        std::string                           m_username;
        std::string                           m_mapname;
        std::string                           m_ServerIP;
        Core&                                 m_Core;
        bool                                  m_Validated;
        uint                                  m_ID;
        double                                m_PingTime;
        bool                                  m_IsCurrentlyConnecting;
        ushort                                m_Port;
        void internalInit(const ushort& server_port, const std::string& server_ipAddress);
    public:
        Client(Core&, const ushort& server_port, const std::string& server_ipAddress, const uint& id);
        ~Client();

        void setClientID(const uint id);

        void changeConnectionDestination(const ushort& port, const std::string& ipAddress);

        const sf::Socket::Status connect(const ushort& timeout = 0);
        void disconnect();
        void onReceive();
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
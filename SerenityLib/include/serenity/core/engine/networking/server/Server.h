#pragma once
#ifndef ENGINE_NETWORKING_SERVER_H
#define ENGINE_NETWORKING_SERVER_H

namespace Engine::Networking {
    class ServerThread;
    class ServerClient;
};

#include <serenity/core/engine/networking/Packet.h>
#include <serenity/core/engine/networking/server/ServerClientsContainer.h>
#include <serenity/core/engine/networking/ListenerTCP.h>
#include <serenity/core/engine/networking/SocketUDP.h>
#include <serenity/core/engine/events/Observer.h>
#include <SFML/Network/Socket.hpp>

enum class ServerType : unsigned char {
    UDP,
    TCP,
};

namespace Engine::Networking {
    class Server : public Observer {
        friend class Engine::Networking::ServerThread;

        using hash_func    = std::function<std::string(sf::IpAddress ip, uint16_t port, sf::Packet packet)>;
        using update_func  = std::function<void(const float dt, bool serverActive)>;
        using on_udp_func  = std::function<void(sf::Packet& sf_packet, std::string& ip, uint16_t port, const float dt)>;

        private:
            hash_func      m_Client_Hash_Function     = [](sf::IpAddress ip, uint16_t port, sf::Packet packet) { return (ip.toString() + "|" + std::to_string(port)); };
            update_func    m_Update_Function          = [](const float dt, bool serverActive) {};
            on_udp_func    m_On_Receive_UDP_Function  = [](sf::Packet& sf_packet, std::string& ip, uint16_t port, const float dt) {};

            void internal_send_to_all_tcp(const ServerClient* exclusion, sf::Packet& packet);
            void internal_send_to_all_udp(const ServerClient* exclusion, sf::Packet& packet);

            void internal_update_tcp_listener_loop(bool serverActive);
            void internal_update_udp_loop(const float dt, bool serverActive);
            void internal_update_client_threads(const float dt, bool serverActive);

            bool internal_add_client(const std::string& hash, ServerClient* client);

            Server() = delete;
        protected:
            ServerClientsContainer                          m_Clients;
            std::mutex                                      m_Mutex;
            std::unique_ptr<SocketUDP>                      m_UdpSocket;
            std::unique_ptr<ListenerTCP>                    m_TCPListener;
            std::string                                     m_IP_Restriction = "";
            ServerType                                      m_ServerType     = ServerType::UDP;
            uint16_t                                        m_Port           = 0;
            std::atomic<bool>                               m_Active         = false;
            bool                                            m_Multithreaded  = true;
        public:
            Server(ServerType type, bool multithreaded = true);
            virtual ~Server();

            inline bool isActive() const noexcept { return m_Active.load(std::memory_order::relaxed); }
            inline explicit operator bool() const noexcept { return isActive(); }

            //override this function with your own custom client class that inherits from Engine::Networking::ServerClient
            virtual ServerClient* add_new_client(std::string& hash, sf::IpAddress clientIP, uint16_t clientPort, SocketTCP* tcp);
            virtual void onEvent(const Event& e) override {}
            virtual bool startup(uint16_t port, std::string ip_restriction = "");
            virtual bool shutdown();

            virtual void clearAllClients();

            ServerClient* getClientFromUDPData(const std::string& ip, uint16_t port, sf::Packet& sf_packet) const;

            inline constexpr unsigned short getPort() const noexcept { return m_Port; }
            inline SocketUDP& getUDPSocket() const noexcept { return *m_UdpSocket.get(); }
            inline constexpr ServerType getType() const noexcept { return m_ServerType; }

            inline size_t num_clients() const noexcept { return m_Clients.size(); }

            inline void setClientHashFunction(hash_func function) { m_Client_Hash_Function = function; }
            inline void setServerUpdateFunction(update_func function) { m_Update_Function = function; }
            inline void setOnReceiveUDPFunction(on_udp_func function) { m_On_Receive_UDP_Function = function; }
            void onReceiveUDP(Engine::Networking::Packet& packet, sf::IpAddress ip, uint16_t port, const float dt);

            void update(const float dt);

            void remove_client(ServerClient& client);
            void remove_client_immediately(ServerClient& client);

            //tcp
            virtual SocketStatus::Status send_tcp_to_client(ServerClient* client, sf::Packet& packet);
            virtual void send_tcp_to_all_but_client(const ServerClient* exclusion, sf::Packet& packet);
            virtual void send_tcp_to_all(sf::Packet& packet);

            //udp
            
            virtual SocketStatus::Status send_udp_to_client(ServerClient* client, sf::Packet& packet);
            virtual void send_udp_to_all_but_client(const ServerClient* exclusion, sf::Packet& packet);
            virtual void send_udp_to_all(sf::Packet& packet);
            
            virtual SocketStatus::Status send_udp_to_client_important(ServerClient* client, Engine::Networking::Packet& packet);
            virtual void send_udp_to_all_but_client_important(const ServerClient* exclusion, Engine::Networking::Packet& packet);
            virtual void send_udp_to_all_important(Engine::Networking::Packet& packet);

            virtual SocketStatus::Status receive_udp(sf::Packet& packet, sf::IpAddress& sender, uint16_t& port);
    };
};
#endif
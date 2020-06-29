#pragma once
#ifndef ENGINE_NETWORKING_SERVER_H
#define ENGINE_NETWORKING_SERVER_H

namespace Engine::Networking {
    class ServerThread;
    class ServerClient;
}

#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <functional>
#include <core/engine/networking/Packet.h>
#include <core/engine/networking/server/ServerThread.h>
#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketUDP.h>
#include <core/engine/events/Observer.h>
#include <SFML/Network/Socket.hpp>

struct ServerType final { enum Type : unsigned char {
    UDP,
    TCP,
};};

namespace Engine::Networking {
    class Server : public Observer, public Engine::NonCopyable, public Engine::NonMoveable {
        friend class Engine::Networking::ServerThread;

        using hash_func              = std::function<std::string(std::string ip, unsigned short port, sf::Packet packet)>;
        using update_func            = std::function<void(const float dt, bool serverActive)>;
        using on_udp_func            = std::function<void(sf::Packet& sf_packet, std::string& ip, unsigned short port, const float dt)>;

        private:
            hash_func              m_Client_Hash_Function       = [](std::string ip, unsigned short port, sf::Packet packet) { return (ip + "|" + std::to_string(port)); };
            update_func            m_Update_Function            = [](const float dt, bool serverActive) {};
            on_udp_func            m_On_Receive_UDP_Function    = [](sf::Packet& sf_packet, std::string& ip, unsigned short port, const float dt) {};

            void internal_send_to_all_tcp(ServerClient* exclusion, sf::Packet& packet);
            //void internal_send_to_all_tcp(ServerClient* exclusion, void* data, size_t size);

            void internal_send_to_all_udp(ServerClient* exclusion, sf::Packet& packet);
            //void internal_send_to_all_udp(ServerClient* exclusion, void* data, size_t size);

            void internal_update_tcp_listener_loop(bool serverActive);
            void internal_update_udp_loop(const float dt, bool serverActive);
            void internal_update_client_threads(const float dt, bool serverActive);
            void internal_update_remove_clients();
            void internal_add_client(std::string& hash, ServerClient& client);

            Server() = delete;
        protected:
            ServerType::Type                                      m_ServerType     = ServerType::UDP;
            std::mutex                                            m_Mutex;

            ServerThreadCollection                                m_Threads;
            std::unordered_map<std::string, ServerClient*>        m_HashedClients;
            std::vector<std::pair<std::string, ServerClient*>>    m_RemovedClients;

            std::unique_ptr<SocketUDP>                            m_UdpSocket      = nullptr;
            std::unique_ptr<ListenerTCP>                          m_TCPListener    = nullptr;
            unsigned short                                        m_Port           = 0;
            std::string                                           m_IP_Restriction = "";
            std::atomic<bool>                                     m_Active         = false;
            bool                                                  m_Multithreaded  = true;
        public:
            Server(ServerType::Type type, bool multithreaded = true);
            virtual ~Server();

            //override this function with your own custom client class that inherits from Engine::Networking::ServerClient
            virtual ServerClient* add_new_client(std::string& hash, std::string& clientIP, unsigned short clientPort, SocketTCP* tcp);
            virtual void onEvent(const Event& e) override {}
            virtual bool startup(unsigned short port, std::string ip_restriction = "");
            virtual bool shutdown();

            ServerClient* getClientFromUDPData(const std::string& ip, unsigned short port, sf::Packet& sf_packet) const;

            constexpr ServerType::Type getType() const noexcept { return m_ServerType; }
            constexpr size_t num_clients() const noexcept { return m_Threads.getNumClients(); }
            void setClientHashFunction(hash_func function) { m_Client_Hash_Function = function; }
            void setServerUpdateFunction(update_func function) { m_Update_Function = function; }
            void setOnReceiveUDPFunction(on_udp_func function) { m_On_Receive_UDP_Function = function; }

            void update(const float dt);

            void remove_client(ServerClient& client);

            //tcp
            SocketStatus::Status send_tcp_to_client(ServerClient& client, sf::Packet& packet);
            void send_tcp_to_all_but_client(ServerClient& exclusion, sf::Packet& packet);
            void send_tcp_to_all(sf::Packet& packet);
            //SocketStatus::Status send_tcp_to_client(ServerClient& client, void* data, size_t size);
            //SocketStatus::Status send_tcp_to_client(ServerClient& client, void* data, size_t size, size_t& sent);
            //void send_tcp_to_all_but_client(ServerClient& exclusion, void* data, size_t size);
            //void send_tcp_to_all(void* data, size_t size);

            //udp
            SocketStatus::Status send_udp_to_client(ServerClient& client, sf::Packet& packet);
            void send_udp_to_all_but_client(ServerClient& exclusion, sf::Packet& packet);
            void send_udp_to_all(sf::Packet& packet);
            SocketStatus::Status receive_udp(sf::Packet& packet, sf::IpAddress& sender, unsigned short& port);
            //void send_udp_to_all_but_client(ServerClient& exclusion, void* data, size_t size);
            //void send_udp_to_all(void* data, size_t size);
            //SocketStatus::Status receive_udp(void* data, size_t size, size_t& received, sf::IpAddress& sender, unsigned short& port);

            //void setBlockingTCPListener(bool blocking);
            //void setBlockingTCPClients(bool blocking);
            //void setBlockingTCPClient(std::string& client_hash, bool blocking);
            //void setBlockingUDP(bool blocking);
    };
};
#endif
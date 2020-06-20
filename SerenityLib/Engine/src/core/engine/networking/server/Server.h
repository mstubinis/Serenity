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
    TCP,
    UDP,
    TCP_AND_UDP,
};};

namespace Engine::Networking {
    class Server : public Observer, public Engine::NonCopyable, public Engine::NonMoveable {
        friend class Engine::Networking::ServerThread;
        private:
            std::function<std::string(std::string ip, unsigned short port, std::string extra)> m_Client_Hash_Function = [=](std::string ip, unsigned short port, std::string extra) { return (ip + "|" + std::to_string(port)) + extra; };
            std::function<void(const float dt, bool serverActive)> m_Update_Function = [=](const float dt, bool serverActive) {};
            std::function<std::string(sf::Packet& packet)> m_Extract_Extra_UDP_Function = [=](sf::Packet& packet) { return ""; };

            void internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, Engine::Networking::Packet& packet);
            void internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, Engine::Networking::Packet& packet);

            void internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, sf::Packet& packet);
            void internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, sf::Packet& packet);

            void internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, void* data, size_t size);
            void internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, void* data, size_t size);

            void internal_update_loop(const float dt, bool serverActive);
            void internal_update_tcp_listener_loop();

            void internal_add_client(std::string& hash, Engine::Networking::ServerClient& client);

            ServerThread* internal_get_next_available_thread();

            Server() = delete;
        protected:
            ServerType::Type                                                          m_ServerType     = ServerType::TCP_AND_UDP;
            std::mutex                                                                m_Mutex;

            std::vector<Engine::Networking::ServerThread>                             m_Threads;
            std::unordered_map<std::string, Engine::Networking::ServerClient*>        m_HashedClients;
            std::vector<std::pair<std::string, Engine::Networking::ServerClient*>>    m_RemovedClients;

            std::unique_ptr<Engine::Networking::SocketUDP>                            m_UdpSocket      = nullptr;
            std::unique_ptr<Engine::Networking::ListenerTCP>                          m_TCPListener    = nullptr;
            unsigned short                                                            m_Port           = 0;
            std::string                                                               m_IP_Restriction = "";
            std::atomic<bool>                                                         m_Active         = false;
        public:
            Server(ServerType::Type type);
            virtual ~Server();

            //override this function with your own custom client class that inherits from Engine::Networking::ServerClient
            virtual Engine::Networking::ServerClient* add_new_client(std::string& hash, std::string& clientIP, unsigned short clientPort, Engine::Networking::SocketTCP* tcp);
            virtual void on_recieve_udp(sf::Packet& sf_packet) {}
            virtual void onEvent(const Event& e) override {}
            virtual bool startup(unsigned short port, std::string ip_restriction = "");
            virtual bool shutdown();
            virtual Engine::Networking::ServerThread* getNextAvailableClientThread();

            ServerType::Type getType() const;

            void update(const float dt);

            void remove_client(Engine::Networking::ServerClient& client);

            void setClientHashFunction(std::function<std::string(std::string ip, unsigned short port, std::string extra)> function);
            void setServerUpdateFunction(std::function<void(const float dt, bool serverActive)> function);
            void setExtraHashExtractUDPFunction(std::function<std::string(sf::Packet& packet)> function);

            void setBlockingTCPListener(bool blocking);
            void setBlockingTCPClients(bool blocking);
            void setBlockingTCPClient(std::string& client_hash, bool blocking);
            void setBlockingUDP(bool blocking);

            unsigned int num_clients() const;
            std::vector<Engine::Networking::ServerClient*> clients() const;

            //tcp
            SocketStatus::Status send_tcp_to_client(Engine::Networking::ServerClient& client, Engine::Networking::Packet& packet);
            SocketStatus::Status send_tcp_to_client(Engine::Networking::ServerClient& client, sf::Packet& packet);
            SocketStatus::Status send_tcp_to_client(Engine::Networking::ServerClient& client, void* data, size_t size);
            SocketStatus::Status send_tcp_to_client(Engine::Networking::ServerClient& client, void* data, size_t size, size_t& sent);
            void send_tcp_to_all_but_client(Engine::Networking::ServerClient& exclusion, Engine::Networking::Packet& packet);
            void send_tcp_to_all_but_client(Engine::Networking::ServerClient& exclusion, sf::Packet& packet);
            void send_tcp_to_all_but_client(Engine::Networking::ServerClient& exclusion, void* data, size_t size);
            void send_tcp_to_all(Engine::Networking::Packet& packet);
            void send_tcp_to_all(sf::Packet& packet);
            void send_tcp_to_all(void* data, size_t size);

            //udp
            void send_udp_to_all_but_client(Engine::Networking::ServerClient& exclusion, Engine::Networking::Packet& packet);
            void send_udp_to_all_but_client(Engine::Networking::ServerClient& exclusion, sf::Packet& packet);
            void send_udp_to_all_but_client(Engine::Networking::ServerClient& exclusion, void* data, size_t size);
            void send_udp_to_all(Engine::Networking::Packet& packet);
            void send_udp_to_all(sf::Packet& packet);
            void send_udp_to_all(void* data, size_t size);
            SocketStatus::Status receive_udp(sf::Packet& packet, sf::IpAddress& sender, unsigned short& port);
            SocketStatus::Status receive_udp(void* data, size_t size, size_t& received, sf::IpAddress& sender, unsigned short& port);
    };
};
#endif
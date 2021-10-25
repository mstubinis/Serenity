#pragma once
#ifndef ENGINE_NETWORKING_SERVER_H
#define ENGINE_NETWORKING_SERVER_H

namespace Engine::Networking {
    class ServerThread;
    class ServerClient;
};

#include <serenity/networking/Packet.h>
#include <serenity/networking/server/ServerClientsContainer.h>
#include <serenity/networking/ListenerTCP.h>
#include <serenity/networking/SocketUDP.h>
#include <serenity/events/Observer.h>
#include <SFML/Network/Socket.hpp>

enum class ServerType : unsigned char {
    UDP,
    TCP,
};

namespace Engine::Networking {
    class Server : public Observer {
        friend class Engine::Networking::ServerThread;

        using hash_func    = std::function<std::string(sf::IpAddress ip, uint16_t port)>;
        using update_func  = std::function<void(const float dt, bool serverActive)>;
        using on_udp_func  = std::function<void(sf::Packet&, std::string& ip, uint16_t port)>;

        private:
            hash_func      m_CustomClientHashFunction    = [](sf::IpAddress ip, uint16_t port) { return (ip.toString() + "|" + std::to_string(port)); };
            update_func    m_CustomUpdateFunction        = [](const float dt, bool serverActive) {};
            on_udp_func    m_CustomOnReceiveUDPFunction  = [](sf::Packet& sf_packet, std::string& ip, uint16_t port) {};

            void internal_send_to_all_clients_tcp(Engine::Networking::Packet&);
            void internal_send_to_all_clients_udp(Engine::Networking::Packet&);
            void internal_send_to_all_clients_but_one_tcp(const ServerClient* const exclusion, Engine::Networking::Packet&);
            void internal_send_to_all_clients_but_one_udp(const ServerClient* const exclusion, Engine::Networking::Packet&);
            bool internal_add_client(const std::string& hash, ServerClient*);
            void internal_on_receive_udp_packet(Engine::Networking::Packet&, sf::IpAddress, uint16_t port);

            Server() = delete;
        protected:
            ServerClientsContainer                          m_Clients;
            std::mutex                                      m_Mutex;
            std::unique_ptr<SocketUDP>                      m_UdpSocket;
            std::unique_ptr<ListenerTCP>                    m_TCPListener;
            std::string                                     m_IP_Restriction;
            ServerType                                      m_ServerType     = ServerType::UDP;
            uint16_t                                        m_Port           = 0;
            std::atomic<bool>                               m_Active         = false;
            bool                                            m_Multithreaded  = true;
        public:
            Server(ServerType type, bool multithreaded = true);
            virtual ~Server();

            [[nodiscard]] inline bool isActive() const noexcept { return m_Active.load(std::memory_order::relaxed); }
            inline explicit operator bool() const noexcept { return isActive(); }

            //override this function with your own custom client class that inherits from Engine::Networking::ServerClient
            virtual ServerClient* addNewClient(const std::string& hash, sf::IpAddress clientIP, uint16_t clientPort, SocketTCP*);
            virtual void onEvent(const Event& e) override {}
            virtual bool startup(uint16_t port, std::string ip_restriction = "");
            virtual bool shutdown();
            virtual void clearAllClients();

            [[nodiscard]] ServerClient* getServerClientFromUDPData(std::string_view clientIP, uint16_t clientPort) const;
            [[nodiscard]] inline constexpr uint16_t getPort() const noexcept { return m_Port; }
            [[nodiscard]] inline SocketUDP& getUDPSocket() const noexcept { return *m_UdpSocket.get(); }
            [[nodiscard]] inline constexpr ServerType getType() const noexcept { return m_ServerType; }
            [[nodiscard]] inline size_t getNumClients() const noexcept { return m_Clients.size(); }

            template<class FUNC> inline void setCustomClientHashFunction(FUNC&& func) noexcept { m_CustomClientHashFunction = std::forward<FUNC>(func); }
            template<class FUNC> inline void setCustomServerUpdateFunction(FUNC&& func) noexcept { m_CustomUpdateFunction = std::forward<FUNC>(func); }
            template<class FUNC> inline void setCustomOnReceiveUDPFunction(FUNC&& func) noexcept { m_CustomOnReceiveUDPFunction = std::forward<FUNC>(func); }

            void update(const float dt);

            void removeClient(ServerClient&);
            void removeClientImmediately(ServerClient&);

            //tcp
            virtual SocketStatus::Status sendTcp(ServerClient*, Engine::Networking::Packet&);
            virtual void sendTcpToAllButOne(const ServerClient* const exclusion, Engine::Networking::Packet&);
            virtual void sendTcpToAll(Engine::Networking::Packet&);

            //udp
            virtual SocketStatus::Status sendUdp(ServerClient*, Engine::Networking::Packet&);
            virtual void sendUdpToAllButOne(const ServerClient* const exclusion, Engine::Networking::Packet&);
            virtual void sendUdpToAll(Engine::Networking::Packet&);

            //per frame update methods
            SocketStatus::Status onUpdateReceiveUDPPackets();
            void onUpdateReceiveTCPPackets(const float dt);
            SocketStatus::Status onUpdateProcessTCPListeners();
    };
};
#endif
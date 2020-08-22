#pragma once
#ifndef ENGINE_NETWORKING_SERVER_CLIENT_H
#define ENGINE_NETWORKING_SERVER_CLIENT_H

namespace Engine::Networking {
    class SocketTCP;
    class Server;
};

#include <SFML/Network/Socket.hpp>
#include <SFML/Network/Packet.hpp>
#include <core/engine/networking/Packet.h>
#include <core/engine/networking/SocketInterface.h>
#include <core/engine/events/Observer.h>

namespace Engine::Networking {
    class ServerClient : public Observer {
        friend class Engine::Networking::Server;

        using update_func                = std::function<void(const float dt)>;
        using on_timed_out_func          = std::function<void()>;
        using on_recovery_timed_out_func = std::function<void()>;
        using on_received_tcp_func       = std::function<void(sf::Packet& sfPacket, const float dt)>;
        using on_received_udp_func       = std::function<void(sf::Packet& sfPacket, const float dt)>;

        public:
            enum class ConnectionState : unsigned char {
                Unknown,
                Active,
                Inactive,
                Disconnected,
            };
        private:
            void internal_update_receive_tcp_packet(const float dt) noexcept;
            void internal_update_connection_state(const float dt) noexcept;
            void internal_on_received_data() noexcept;
            void internal_on_receive_udp(sf::Packet& packet, const float dt) noexcept;

            on_timed_out_func          m_On_Timed_Out_Function          = []() {};
            on_recovery_timed_out_func m_On_Recovery_Timed_Out_Function = []() {};
            update_func                m_Update_Function                = [](const float dt) {};
            on_received_tcp_func       m_On_Received_TCP_Function       = [](sf::Packet& sfPacket, const float dt) {};
            on_received_udp_func       m_On_Received_UDP_Function       = [](sf::Packet& sfPacket, const float dt) {};
        protected:
            Engine::Networking::Server&                             m_Server;
            ConnectionState                                         m_ConnectionState              = ConnectionState::Unknown;
            mutable std::unique_ptr<Engine::Networking::SocketTCP>  m_TcpSocket                    = nullptr;
            sf::IpAddress                                           m_IP                           = sf::IpAddress::LocalHost;
            std::string                                             m_Hash                         = "";
            unsigned short                                          m_Port                         = 0U;
            std::uint32_t                                           m_ID                           = 0U;
            float                                                   m_Timeout_Timer                = 0.0f;
            float                                                   m_Timeout_Timer_Limit          = 30.0f;
            float                                                   m_Recovery_Timeout_Timer       = 0.0f;
            float                                                   m_Recovery_Timeout_Timer_Limit = 60.0f;
        public:
            ServerClient(const std::string& hash, Engine::Networking::Server& server, Engine::Networking::SocketTCP* tcp, const std::string& clientIP, unsigned short clientPort);
            virtual ~ServerClient();
      
            void setOnUpdateFunction(update_func function) noexcept { m_Update_Function = function; }
            void setOnTimedOutFunction(on_timed_out_func function) noexcept { m_On_Timed_Out_Function = function; }
            void setOnRecoveryTimedOutFunction(on_recovery_timed_out_func function) noexcept { m_On_Recovery_Timed_Out_Function = function; }
            void setOnReceivedTCPFunction(on_received_tcp_func function) noexcept { m_On_Received_TCP_Function = function; }
            void setOnReceivedUDPFunction(on_received_udp_func function) noexcept { m_On_Received_UDP_Function = function; }
            void setUpdateFunction(update_func function) noexcept { m_Update_Function = function; }

            virtual void onEvent(const Event& e) override {}

            inline CONSTEXPR ConnectionState connectionState() const noexcept { return m_ConnectionState; }
            inline CONSTEXPR const std::string& hash() const noexcept { return m_Hash; }
            inline CONSTEXPR Engine::Networking::SocketTCP* socket() const noexcept { return m_TcpSocket.get(); }
            inline CONSTEXPR unsigned short port() const noexcept { return m_Port; }
            inline CONSTEXPR const sf::IpAddress& ip() const noexcept { return m_IP; }
            inline CONSTEXPR std::uint32_t id() const noexcept { return m_ID; }
            inline CONSTEXPR void setTimeoutTimerLimit(float limit) noexcept { m_Timeout_Timer_Limit = limit; }
            inline CONSTEXPR void setRecoveryTimeoutTimerLimit(float limit) noexcept { m_Recovery_Timeout_Timer_Limit = limit; }

            sf::Uint32 generate_nonce() const noexcept;

            void disconnect() noexcept;
            bool connect(unsigned short timeout = 0) noexcept;
            bool disconnected() const noexcept;

            SocketStatus::Status send_udp(sf::Packet& sfPacket) noexcept;
            SocketStatus::Status send_tcp(sf::Packet& sfPacket) noexcept;
            SocketStatus::Status receive_tcp(sf::Packet& packet) noexcept;

            void update(const float dt) noexcept;
    };
};
#endif
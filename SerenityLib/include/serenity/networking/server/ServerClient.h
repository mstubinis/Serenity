#pragma once
#ifndef ENGINE_NETWORKING_SERVER_CLIENT_H
#define ENGINE_NETWORKING_SERVER_CLIENT_H

namespace Engine::Networking {
    class SocketTCP;
    class Server;
};

#include <SFML/Network/Socket.hpp>
#include <SFML/Network/Packet.hpp>
#include <serenity/networking/Packet.h>
#include <serenity/networking/ISocket.h>
#include <serenity/events/Observer.h>

namespace Engine::Networking {
    class ServerClient : public Observer {
        friend class Engine::Networking::Server;

        using update_func                = std::function<void(const float dt)>;
        using on_timed_out_func          = std::function<void()>;
        using on_recovery_timed_out_func = std::function<void()>;
        using on_received_tcp_func       = std::function<void(sf::Packet& sfPacket)>;
        using on_received_udp_func       = std::function<void(sf::Packet& sfPacket)>;

        public:
            enum class ConnectionState : uint8_t {
                Unknown,
                Active,
                Inactive,
                Disconnected,
            };
        private:
            void internal_update_receive_tcp_packet() noexcept;
            void internal_update_connection_state(const float dt) noexcept;
            void internal_on_received_data() noexcept;
            void internal_on_receive_udp(sf::Packet& packet) noexcept;

            on_timed_out_func          m_On_Timed_Out_Function          = []() {};
            on_recovery_timed_out_func m_On_Recovery_Timed_Out_Function = []() {};
            update_func                m_Update_Function                = [](const float dt) {};
            on_received_tcp_func       m_On_Received_TCP_Function       = [](sf::Packet& sfPacket) {};
            on_received_udp_func       m_On_Received_UDP_Function       = [](sf::Packet& sfPacket) {};
        protected:
            mutable std::unique_ptr<Engine::Networking::SocketTCP>  m_TcpSocket;
            Engine::Networking::Server&                             m_Server;
            sf::IpAddress                                           m_IP                           = sf::IpAddress::LocalHost;
            std::string                                             m_Hash;
            uint32_t                                                m_ID                           = 0;
            ConnectionState                                         m_ConnectionState              = ConnectionState::Unknown;
            float                                                   m_Timeout_Timer                = 0.0f;
            float                                                   m_Timeout_Timer_Limit          = 30.0f;
            float                                                   m_Recovery_Timeout_Timer       = 0.0f;
            float                                                   m_Recovery_Timeout_Timer_Limit = 60.0f;
            uint16_t                                                m_Port                         = 0;
        public:
            ServerClient(const std::string& hash, Engine::Networking::Server& server, Engine::Networking::SocketTCP* tcp, sf::IpAddress clientIP, uint16_t clientPort);
            virtual ~ServerClient();
      
            ServerClient(const ServerClient& other)                 = delete;
            ServerClient& operator=(const ServerClient& other)      = delete;
            ServerClient(ServerClient&& other) noexcept             = default;
            ServerClient& operator=(ServerClient&& other) noexcept  = default;

            inline void setOnUpdateFunction(update_func&& function) noexcept { m_Update_Function = std::move(function); }
            inline void setOnTimedOutFunction(on_timed_out_func&& function) noexcept { m_On_Timed_Out_Function = std::move(function); }
            inline void setOnRecoveryTimedOutFunction(on_recovery_timed_out_func&& function) noexcept { m_On_Recovery_Timed_Out_Function = std::move(function); }
            inline void setOnReceivedTCPFunction(on_received_tcp_func&& function) noexcept { m_On_Received_TCP_Function = std::move(function); }
            inline void setOnReceivedUDPFunction(on_received_udp_func&& function) noexcept { m_On_Received_UDP_Function = std::move(function); }
            inline void setUpdateFunction(update_func&& function) noexcept { m_Update_Function = std::move(function); }

            virtual void onEvent(const Event& e) override {}

            [[nodiscard]] inline constexpr ConnectionState connectionState() const noexcept { return m_ConnectionState; }
            [[nodiscard]] inline constexpr const std::string& hash() const noexcept { return m_Hash; }
            [[nodiscard]] inline Engine::Networking::SocketTCP* socket() const noexcept { return m_TcpSocket.get(); }
            [[nodiscard]] inline constexpr uint16_t port() const noexcept { return m_Port; }
            [[nodiscard]] inline sf::IpAddress ip() const noexcept { return m_IP; }
            [[nodiscard]] inline constexpr uint32_t id() const noexcept { return m_ID; }
            inline constexpr void setTimeoutTimerLimit(float limit) noexcept { m_Timeout_Timer_Limit = limit; }
            inline constexpr void setRecoveryTimeoutTimerLimit(float limit) noexcept { m_Recovery_Timeout_Timer_Limit = limit; }

            [[nodiscard]] uint32_t generate_nonce() const noexcept;

            void disconnect() noexcept;
            bool connect(uint16_t timeout = 0) noexcept;
            bool disconnected() const noexcept;

            SocketStatus::Status send_udp(sf::Packet& sfPacket) noexcept;
            SocketStatus::Status send_tcp(sf::Packet& sfPacket) noexcept;
            SocketStatus::Status receive_tcp(sf::Packet& packet) noexcept;

            void update(const float dt) noexcept;
    };
};
#endif
#pragma once
#ifndef ENGINE_NETWORKING_SERVER_CLIENT_H
#define ENGINE_NETWORKING_SERVER_CLIENT_H

namespace Engine::Networking {
    class SocketTCP;
    class Server;
};
#include <string>
#include <functional>
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

        public:
            struct ConnectionState final { enum State : unsigned char {
                Unknown,
                Active,
                Inactive,
                Disconnected,
            };};
        private:
            void internal_update_tcp(const float dt);
            void internal_update_connection_state(const float dt);
            void internal_on_received_data();

            on_timed_out_func          m_On_Timed_Out_Function          = []() {};
            on_recovery_timed_out_func m_On_Recovery_Timed_Out_Function = []() {};
            update_func                m_Update_Function                = [](const float dt) {};
            on_received_tcp_func       m_On_Received_TCP_Function       = [](sf::Packet& sfPacket, const float dt) {};

        protected:
            Engine::Networking::Server&             m_Server;
            ConnectionState::State                  m_ConnectionState              = ConnectionState::Unknown;
            mutable Engine::Networking::SocketTCP*  m_TcpSocket                    = nullptr;
            std::string                             m_IP                           = "";
            std::string                             m_Hash                         = "";
            unsigned short                          m_Port                         = 0;
            unsigned int                            m_ID                           = 0;
            float                                   m_Timeout_Timer                = 0.0f;
            float                                   m_Timeout_Timer_Limit          = 30.0f;
            float                                   m_Recovery_Timeout_Timer       = 0.0f;
            float                                   m_Recovery_Timeout_Timer_Limit = 60.0f;
        public:
            ServerClient(std::string& hash, Engine::Networking::Server& server, Engine::Networking::SocketTCP* tcp, std::string& clientIP, unsigned short clientPort);
            virtual ~ServerClient();
      
            void setOnUpdateFunction(update_func function) { m_Update_Function = function; }
            void setOnTimedOutFunction(on_timed_out_func function) { m_On_Timed_Out_Function = function; }
            void setOnRecoveryTimedOutFunction(on_recovery_timed_out_func function) { m_On_Recovery_Timed_Out_Function = function; }
            void setOnReceivedTCPFunction(on_received_tcp_func function) { m_On_Received_TCP_Function = function; }

            virtual void on_receive_udp(sf::Packet& sfPacket, const float dt) {}
            //virtual void on_receive_udp(void* data, size_t size, size_t& received, const float dt) {}
            virtual void onEvent(const Event& e) override {}

            constexpr ConnectionState::State connectionState() const noexcept { return m_ConnectionState; }
            constexpr const std::string& hash() const noexcept { return m_Hash; }
            constexpr Engine::Networking::SocketTCP* socket() const noexcept { return m_TcpSocket; }
            constexpr unsigned short port() const noexcept { return m_Port; }
            constexpr const std::string& ip() const noexcept { return m_IP; }
            constexpr unsigned int id() const noexcept { return m_ID; }
            constexpr void setTimeoutTimerLimit(float limit) { m_Timeout_Timer_Limit = limit; }
            void setUpdateFunction(update_func function) { m_Update_Function = function; }

            sf::Uint32 generate_nonce() const;

            void disconnect();
            bool connect(unsigned short timeout = 0);
            bool disconnected() const;

            SocketStatus::Status send_tcp(sf::Packet& sfPacket);
            //SocketStatus::Status send_tcp(void* data, size_t size);
            //SocketStatus::Status send_tcp(void* data, size_t size, size_t& sent);
            SocketStatus::Status receive_tcp(sf::Packet& packet);
            //SocketStatus::Status receive_tcp(void* data, size_t size, size_t& received);

            SocketStatus::Status send_udp(sf::Packet& sfPacket);
            void receive_udp(SocketStatus::Status status, sf::Packet& packet, const float dt);
            //void receive_udp(SocketStatus::Status status, void* data, size_t size, size_t& received, const float dt);

            void update(const float dt);
    };
};
#endif
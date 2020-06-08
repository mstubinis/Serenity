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
        public:
            struct ConnectionState final { enum State : unsigned char {
                Active,
                Inactive,
                Disconnected,
            };};
        private:
            void internal_update_loop(const float dt);
            void internal_on_received_data();
        protected:
            Engine::Networking::Server&             m_Server;
            ConnectionState::State                  m_ConnectionState              = ConnectionState::Active;
            mutable Engine::Networking::SocketTCP*  m_TcpSocket                    = nullptr;
            std::function<void(const float dt)>     m_Update_Function              = [=](const float dt) {};
            std::string                             m_IP                           = "";
            std::string                             m_Hash                         = "";
            unsigned short                          m_Port                         = 0;
            unsigned int                            m_ID                           = 0;
            float                                   m_Timeout_Timer                = 0.0f;
            float                                   m_Timeout_Timer_Limit          = 30.0f;
            float                                   m_Recovery_Timeout_Timer       = 0.0f;
            float                                   m_Recovery_Timeout_Timer_Limit = 60.0f;
        public:
            ServerClient(const std::string& hash, Engine::Networking::Server& server, Engine::Networking::SocketTCP* tcp);
            virtual ~ServerClient();
      
            ConnectionState::State connectionState() const;
            Engine::Networking::SocketTCP* socket() const;
            unsigned short port() const;
            const std::string& ip() const;
            const std::string& hash() const;
            unsigned int id() const;
            sf::Uint32 generate_nonce() const;

            void disconnect();
            bool connect(unsigned short timeout = 0);
            bool disconnected() const;
            void setTimeoutTimerLimit(float limit);

            SocketStatus::Status send(Engine::Networking::Packet& packet);
            SocketStatus::Status send(sf::Packet& sfPacket);
            SocketStatus::Status send(void* data, size_t size);
            SocketStatus::Status send(void* data, size_t size, size_t& sent);
            SocketStatus::Status receive(sf::Packet& packet);
            SocketStatus::Status receive(void* data, size_t size, size_t& received);

            virtual void on_receive_tcp(sf::Packet& sfPacket, const float dt);
            virtual void on_timed_out();
            virtual void on_recovery_timed_out();
            virtual void onEvent(const Event& e) override;

            void setUpdateFunction(std::function<void(const float dt)> function);
            void update(const float dt);
    };
};
#endif
#pragma once
#ifndef GAME_NETWORKING_AUTHENTICATION_LAYER_H
#define GAME_NETWORKING_AUTHENTICATION_LAYER_H

class  Server;
struct Packet;
class  ServerClientThread;
class  ServerClient;

#include <string>
#include <unordered_map>
#include <vector>
#include <core/engine/networking/SocketTCP.h>

class AuthenticationLayer final {
    friend class Server;
    public: 
        class AuthenticationInstance final {
            friend class ::AuthenticationLayer;
            private:
                std::string                         m_Client_IP;
                Server&                             m_Server;
                Engine::Networking::SocketTCP*      m_TcpSocket;
                double                              m_Timer;
                sf::Uint32                          m_Nonce;
                sf::Uint32                          m_SecretKey;

                void end_this_instance();
                void end_auth_process(const std::string& real_username);
                void move_client_to_game(ServerClient& serverClient, ServerClientThread& thread, const std::string& real_username);
            public:
                AuthenticationInstance(Server& server, sf::TcpSocket* socket, const std::string& client_ip);
                ~AuthenticationInstance();

                AuthenticationInstance(const AuthenticationInstance& other)                = delete;
                AuthenticationInstance& operator=(const AuthenticationInstance& other)     = delete;
                AuthenticationInstance(AuthenticationInstance&& other) noexcept            = delete;
                AuthenticationInstance& operator=(AuthenticationInstance&& other) noexcept = delete;

                const sf::Socket::Status send(Packet& packet);

                const sf::Uint32 generate_nonce() const;

                void update(const double& dt);

        };
    private:
        Server&                                                  m_Server;
        std::unordered_map<std::string, AuthenticationInstance>  m_Instances;
    public:
        AuthenticationLayer(Server& server);
        ~AuthenticationLayer();

        AuthenticationLayer(const AuthenticationLayer& other)                = delete;
        AuthenticationLayer& operator=(const AuthenticationLayer& other)     = delete;
        AuthenticationLayer(AuthenticationLayer&& other) noexcept            = delete;
        AuthenticationLayer& operator=(AuthenticationLayer&& other) noexcept = delete;

        const bool create_instance(const std::string& client_ip, sf::TcpSocket* socket);
        const bool allow_host_to_pass(const std::string& real_username);

        void update(const double& dt);

};

#endif
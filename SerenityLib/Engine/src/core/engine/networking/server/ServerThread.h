#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_H
#define ENGINE_NETWORKING_SERVER_THREAD_H

#include <unordered_map>
#include <string>

namespace Engine::Networking {
    class Server;
    class ServerClient;
};
namespace Engine::Networking {
    class ServerThread : public Engine::NonCopyable {
        protected:
            mutable std::unordered_map<std::string, Engine::Networking::ServerClient*>  m_Clients;
        public:
            ServerThread();
            virtual ~ServerThread();

            ServerThread(ServerThread&& other) noexcept;
            ServerThread& operator=(ServerThread&& other) noexcept;

            bool remove_client(const std::string& hash, Engine::Networking::Server& server);
            bool add_client(std::string& hash, Engine::Networking::ServerClient* client, Engine::Networking::Server& server);
            unsigned int num_clients() const;
            std::unordered_map<std::string, Engine::Networking::ServerClient*>& clients() const;
    };
};

#endif
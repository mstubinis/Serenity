#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_H
#define ENGINE_NETWORKING_SERVER_THREAD_H

namespace Engine::Networking {
    class ServerClient;
    class ServerThreadCollection;
};
namespace Engine::Networking {
    class ServerThread final {
        friend class ServerThreadCollection;
        using ClientsMap = std::unordered_map<std::string, ServerClient*>;
        protected:
            ClientsMap  m_HashedServerClients;
        public:
            ServerThread()                                         = default;
            ~ServerThread();
            ServerThread(const ServerThread& other)                = default;
            ServerThread& operator=(const ServerThread& other)     = default;
            ServerThread(ServerThread&& other) noexcept            = default;
            ServerThread& operator=(ServerThread&& other) noexcept = default;

            void remove_all_clients() noexcept { m_HashedServerClients.clear(); }
            inline bool has(const std::string& hash) noexcept { return m_HashedServerClients.count(hash); }
            bool remove_client(const std::string& hash);
            bool add_client(const std::string& hash, ServerClient* client);

            inline size_t num_clients() const noexcept { return m_HashedServerClients.size(); }
            inline ClientsMap& clients() noexcept { return m_HashedServerClients; }

            inline ClientsMap::iterator begin() noexcept { return m_HashedServerClients.begin(); }
            inline ClientsMap::const_iterator begin() const noexcept { return m_HashedServerClients.begin(); }
            inline ClientsMap::iterator end() noexcept { return m_HashedServerClients.end(); }
            inline ClientsMap::const_iterator end() const noexcept { return m_HashedServerClients.end(); }
    };
};


#endif
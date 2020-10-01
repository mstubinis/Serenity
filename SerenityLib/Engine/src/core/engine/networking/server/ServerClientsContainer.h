#pragma once
#ifndef ENGINE_NETWORKING_SERVER_CLIENTS_CONTAINER_H
#define ENGINE_NETWORKING_SERVER_CLIENTS_CONTAINER_H

namespace Engine::Networking {
    class Server;
};

#include <core/engine/networking/server/ServerThreadCollection.h>

namespace Engine::Networking {
    class ServerClientsContainer final {
        friend class Server;
        private:
            ServerThreadCollection                                m_ServerClientThreadsCollection;
            std::unordered_map<std::string, ServerClient*>        m_HashedClients;
            std::vector<std::pair<std::string, ServerClient*>>    m_RemovedClients;

            void internal_update_remove_clients() noexcept;

            ServerClientsContainer() = delete;
        public:
            ServerClientsContainer(size_t numThreads);
            
            inline size_t getNumClients() const noexcept { return m_ServerClientThreadsCollection.getNumClients(); }
            inline size_t size() const noexcept { return m_HashedClients.size(); }

            ServerClient* get(const std::string& hash) const noexcept;
            inline bool has(const std::string& hash) const noexcept { return m_HashedClients.count(hash); }

            bool remove_client(const std::string& hash) noexcept;
            bool remove_client_delayed(ServerClient& inClient, std::mutex& inMutex) noexcept;
            bool add_client(const std::string& hash, ServerClient* serverClient);

            void clear() noexcept {
                for (auto& thread : m_ServerClientThreadsCollection) {
                    thread.remove_all_clients();
                }
                m_HashedClients.clear();
            }

            inline ServerThreadCollection& getThreads() noexcept { return m_ServerClientThreadsCollection; }

            inline std::unordered_map<std::string, ServerClient*>::iterator begin() noexcept { return m_HashedClients.begin(); }
            inline std::unordered_map<std::string, ServerClient*>::const_iterator begin() const noexcept { return m_HashedClients.begin(); }
            inline std::unordered_map<std::string, ServerClient*>::iterator end() noexcept { return m_HashedClients.end(); }
            inline std::unordered_map<std::string, ServerClient*>::const_iterator end() const noexcept { return m_HashedClients.end(); }
    };
};

#endif
#pragma once
#ifndef ENGINE_NETWORKING_SERVER_CLIENTS_CONTAINER_H
#define ENGINE_NETWORKING_SERVER_CLIENTS_CONTAINER_H

#include <serenity/networking/server/ServerThread.h>
#include <serenity/utils/Utils.h>
#include <mutex>
#include <serenity/system/Macros.h>

namespace Engine::Networking {
    class ServerClientsContainer final {
        friend class Server;
        using HashMap       = Engine::unordered_string_map<std::string, ServerClient*>;
        using RemovedVector = std::vector<std::pair<std::string, ServerClient*>>;
        private:
            ServerThreadContainer     m_ThreadContainer;
            HashMap                   m_HashedClients;
            RemovedVector             m_RemovedClients;

            void internal_update_remove_clients() noexcept;

            ServerClientsContainer() = delete;
        public:
            ServerClientsContainer(size_t numThreads);

            [[nodiscard]] inline ServerThreadContainer& getThreads() noexcept { return m_ThreadContainer; }
            [[nodiscard]] inline ServerClient* getClient(std::string_view hash) const noexcept { return (m_HashedClients.contains(hash)) ? m_HashedClients.find(hash)->second : nullptr; }
            [[nodiscard]] bool contains(std::string_view hash) const noexcept { return m_HashedClients.contains(hash); }
            [[nodiscard]] inline size_t size() const noexcept { return m_HashedClients.size(); }

            bool addClient(std::string_view hash, ServerClient* client);
            bool removeClient(ServerClient& inClient, std::mutex& inMutex);
            bool removeClientImmediately(ServerClient& inClient, std::mutex& inMutex);
            void clearAllClients();

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(HashMap, m_HashedClients)
    };
};

#endif
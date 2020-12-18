#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_COLLECTION_H
#define ENGINE_NETWORKING_SERVER_THREAD_COLLECTION_H

namespace Engine::Networking {
    class ServerThreadCollection;
    class ServerThreadContainer;
    class ServerThread;
};

#include <serenity/core/engine/networking/server/ServerThread.h>

namespace Engine::Networking {
    class ServerThreadCollection {
        private:
            std::vector<ServerThread>   m_ServerClientThreads;
            size_t                      m_NumClients = 0;

            ServerThreadCollection() = delete;
        public:
            ServerThreadCollection(size_t threadCount);
            ServerThreadCollection(const ServerThreadCollection& other) = delete;
            ServerThreadCollection& operator=(const ServerThreadCollection& other) = delete;
            ServerThreadCollection(ServerThreadCollection&& other) noexcept = default;
            ServerThreadCollection& operator=(ServerThreadCollection&& other) noexcept = default;

            void setBlocking(bool blocking);
            void setBlocking(const std::string& hash, bool blocking);

            bool has(const std::string& hash) noexcept {
                for (auto& thread : m_ServerClientThreads) {
                    if (thread.has(hash)) {
                        return true;
                    }
                }
                return false;
            }

            bool addClient(const std::string& hash, ServerClient* client);
            bool removeClient(const std::string& hash);
            void clear() { m_ServerClientThreads.clear(); }

            inline size_t getNumClients() const noexcept { return m_NumClients; }
            ServerThread* getNextAvailableClientThread();

            std::vector<ServerThread>::iterator begin() noexcept { return m_ServerClientThreads.begin(); }
            std::vector<ServerThread>::iterator end() noexcept { return m_ServerClientThreads.end(); }
            std::vector<ServerThread>::const_iterator begin() const noexcept { return m_ServerClientThreads.begin(); }
            std::vector<ServerThread>::const_iterator end() const noexcept { return m_ServerClientThreads.end(); }
        };
};

#endif
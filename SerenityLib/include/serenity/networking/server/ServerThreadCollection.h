#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_COLLECTION_H
#define ENGINE_NETWORKING_SERVER_THREAD_COLLECTION_H

namespace Engine::Networking {
    class ServerThreadCollection;
    class ServerThreadContainer;
    class ServerThread;
};

#include <serenity/networking/server/ServerThread.h>

namespace Engine::Networking {
    class ServerThreadCollection {
        private:
            std::vector<ServerThread>   m_ServerClientThreads;
            size_t                      m_NumClients = 0;

            ServerThreadCollection() = delete;
        public:
            ServerThreadCollection(size_t threadCount);
            ServerThreadCollection(const ServerThreadCollection&)                = delete;
            ServerThreadCollection& operator=(const ServerThreadCollection&)     = delete;
            ServerThreadCollection(ServerThreadCollection&&) noexcept            = default;
            ServerThreadCollection& operator=(ServerThreadCollection&&) noexcept = default;

            void setBlocking(bool blocking);
            void setBlocking(std::string_view hash, bool blocking);

            [[nodiscard]] bool has(std::string_view hash) noexcept {
                for (auto& thread : m_ServerClientThreads) {
                    if (thread.has(hash)) {
                        return true;
                    }
                }
                return false;
            }

            bool addClient(std::string_view hash, ServerClient* client);
            bool removeClient(std::string_view hash);
            inline void clear() { m_ServerClientThreads.clear(); }

            [[nodiscard]] inline size_t getNumClients() const noexcept { return m_NumClients; }
            [[nodiscard]] ServerThread* getNextAvailableClientThread();

            inline std::vector<ServerThread>::iterator begin() noexcept { return m_ServerClientThreads.begin(); }
            inline std::vector<ServerThread>::iterator end() noexcept { return m_ServerClientThreads.end(); }
            inline std::vector<ServerThread>::const_iterator begin() const noexcept { return m_ServerClientThreads.begin(); }
            inline std::vector<ServerThread>::const_iterator end() const noexcept { return m_ServerClientThreads.end(); }
        };
};

#endif
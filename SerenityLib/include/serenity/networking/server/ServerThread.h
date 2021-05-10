#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_H
#define ENGINE_NETWORKING_SERVER_THREAD_H

namespace Engine::Networking {
    class ServerClient;
    class ServerThreadContainer;
    class ServerThreadCollection;
    class ServerThread;
};

#include <serenity/utils/Utils.h>
#include <serenity/system/Macros.h>
#include <vector>
#include <memory>
#include <string>

namespace Engine::Networking {
    class ServerThread final {
        friend class ServerThreadContainer;
        friend class ServerThreadCollection;
        using ClientContainer = Engine::unordered_string_map<std::string, std::unique_ptr<ServerClient>>;
        protected:
            mutable ClientContainer  m_HashedServerClients;
        public:
            ServerThread() = default;
            ServerThread(const ServerThread&)                = delete;
            ServerThread& operator=(const ServerThread&)     = delete;
            ServerThread(ServerThread&&) noexcept            = default;
            ServerThread& operator=(ServerThread&&) noexcept = default;

            void clearAllClients();
            bool removeClient(std::string_view hash);
            bool addClient(std::string_view hash, ServerClient* client);
            [[nodiscard]] inline bool hasClient(std::string_view hash) const noexcept { return m_HashedServerClients.contains(hash); }

            [[nodiscard]] inline size_t getNumClients() const noexcept { return m_HashedServerClients.size(); }
            [[nodiscard]] inline ClientContainer& getClients() const noexcept { return m_HashedServerClients; }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(ClientContainer, m_HashedServerClients)
    };

    class ServerThreadContainer {
        friend class ServerThread;
        using ThreadContainer = std::vector<ServerThread>;
        private:
            ThreadContainer  m_Threads;
            size_t           m_NumClients = 0;
        public:
            ServerThreadContainer(size_t threadCount);

            void setBlocking(bool blocking);
            void setBlocking(std::string_view hash, bool blocking);

            void removeAllClients();
            bool addClient(std::string_view hash, ServerClient* client);
            bool removeClient(std::string_view hash);

            [[nodiscard]] inline constexpr size_t getNumClients() const noexcept { return m_NumClients; }
            [[nodiscard]] ServerThread* getNextAvailableClientThread();

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(ThreadContainer, m_Threads)
    };
};


#endif
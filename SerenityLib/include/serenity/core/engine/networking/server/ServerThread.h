#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_H
#define ENGINE_NETWORKING_SERVER_THREAD_H

namespace Engine::Networking {
    class ServerClient;
    class ServerThreadContainer;
    class ServerThreadCollection;
    class ServerThread;
};

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine::Networking {
    class ServerThread final {
        friend class ServerThreadContainer;
        friend class ServerThreadCollection;
        using ClientContainer = std::unordered_map<std::string, std::unique_ptr<ServerClient>>;
        protected:
            mutable ClientContainer  m_HashedServerClients;
        public:
            ServerThread() = default;
            ServerThread(const ServerThread&) = delete;
            ServerThread& operator=(const ServerThread&) = delete;
            ServerThread(ServerThread&&) noexcept;
            ServerThread& operator=(ServerThread&&) noexcept;

            void clearAllClients();
            bool remove_client(const std::string& hash);
            bool add_client(const std::string& hash, ServerClient* client);
            inline bool has(const std::string& hash) const noexcept { return m_HashedServerClients.contains(hash); }

            inline size_t num_clients() const noexcept { return m_HashedServerClients.size(); }
            inline ClientContainer& clients() const noexcept { return m_HashedServerClients; }

            inline ClientContainer::iterator begin() noexcept { return m_HashedServerClients.begin(); }
            inline ClientContainer::iterator end() noexcept { return m_HashedServerClients.end(); }
            inline ClientContainer::const_iterator begin() const noexcept { return m_HashedServerClients.begin(); }
            inline ClientContainer::const_iterator end() const noexcept { return m_HashedServerClients.end(); }
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
            void setBlocking(const std::string& hash, bool blocking);

            void removeAllClients();
            bool addClient(const std::string& hash, ServerClient* client);
            bool removeClient(const std::string& hash);

            inline constexpr size_t getNumClients() const noexcept { return m_NumClients; }
            ServerThread* getNextAvailableClientThread();

            inline ThreadContainer::iterator begin() noexcept { return m_Threads.begin(); }
            inline ThreadContainer::iterator end() noexcept { return m_Threads.end(); }
            inline ThreadContainer::const_iterator begin() const noexcept { return m_Threads.begin(); }
            inline ThreadContainer::const_iterator end() const noexcept { return m_Threads.end(); }
    };
};


#endif
#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_H
#define ENGINE_NETWORKING_SERVER_THREAD_H

namespace Engine::Networking {
    class Server;
    class ServerClient;
    class ServerThreadContainer;
};
namespace Engine::Networking {
    class ServerThread final : public Engine::NonCopyable, public Engine::NonMoveable {
        friend class ServerThreadContainer;
        protected:
            mutable std::unordered_map<std::string, std::unique_ptr<ServerClient>>  m_ServerClients;
        public:
            ServerThread() = default;
            ServerThread(ServerThread&& other) noexcept;
            ServerThread& operator=(ServerThread&& other) noexcept;

            void clearAllClients();
            bool remove_client(const std::string& hash);
            bool add_client(const std::string& hash, ServerClient* client);

            inline CONSTEXPR size_t num_clients() const noexcept { return m_ServerClients.size(); }
            inline CONSTEXPR std::unordered_map<std::string, std::unique_ptr<ServerClient>>& clients() const noexcept { return m_ServerClients; }
    };

    class ServerThreadContainer {
        private:
            std::vector<ServerThread> m_Threads;
            size_t                    m_NumClients = 0;
        public:
            ServerThreadContainer(size_t threadCount);

            void setBlocking(bool blocking);
            void setBlocking(const std::string& hash, bool blocking);

            void removeAllClients();
            bool addClient(const std::string& hash, ServerClient* client);
            bool removeClient(const std::string& hash);

            inline CONSTEXPR size_t getNumClients() const noexcept { return m_NumClients; }
            ServerThread* getNextAvailableClientThread();

            std::vector<ServerThread>::iterator begin() noexcept { return m_Threads.begin(); }
            std::vector<ServerThread>::iterator end() noexcept { return m_Threads.end(); }
            std::vector<ServerThread>::const_iterator begin() const noexcept { return m_Threads.begin(); }
            std::vector<ServerThread>::const_iterator end() const noexcept { return m_Threads.end(); }
    };
};


#endif
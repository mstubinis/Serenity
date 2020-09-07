#pragma once
#ifndef ENGINE_NETWORKING_SERVER_THREAD_H
#define ENGINE_NETWORKING_SERVER_THREAD_H

namespace Engine::Networking {
    class Server;
    class ServerClient;
};
namespace Engine::Networking {
    class ServerThreadCollection;
    class ServerThread final : public Engine::NonCopyable, public Engine::NonMoveable {
        friend class ServerThreadCollection;
        protected:
            mutable std::unordered_map<std::string, std::unique_ptr<ServerClient>>  m_ServerClients;
        public:
            ServerThread();
            ~ServerThread();

            ServerThread(ServerThread&& other) noexcept;
            ServerThread& operator=(ServerThread&& other) noexcept;

            bool remove_client(const std::string& hash, Server& server);
            bool add_client(const std::string& hash, ServerClient* client, Server& server);

            inline CONSTEXPR size_t num_clients() const noexcept { return m_ServerClients.size(); }
            inline CONSTEXPR std::unordered_map<std::string, std::unique_ptr<ServerClient>>& clients() const noexcept { return m_ServerClients; }
    };

    class ServerThreadCollection {
        private:
            std::vector<ServerThread> m_Threads;
            size_t                    m_NumClients = 0;
        public:
            ServerThreadCollection(size_t threadCount);
            ~ServerThreadCollection();

            void setBlocking(bool blocking);
            void setBlocking(const std::string& hash, bool blocking);

            bool addClient(const std::string& hash, ServerClient* client, Server& server);
            bool removeClient(const std::string& hash, Server& server);

            inline CONSTEXPR size_t getNumClients() const noexcept { return m_NumClients; }
            ServerThread* getNextAvailableClientThread();

            std::vector<ServerThread>::iterator begin() noexcept { return m_Threads.begin(); }
            std::vector<ServerThread>::iterator end() noexcept { return m_Threads.end(); }
            std::vector<ServerThread>::const_iterator begin() const noexcept { return m_Threads.begin(); }
            std::vector<ServerThread>::const_iterator end() const noexcept { return m_Threads.end(); }
    };
};


#endif
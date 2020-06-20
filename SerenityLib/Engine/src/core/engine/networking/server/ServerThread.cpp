#include <core/engine/networking/server/ServerThread.h>
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/server/Server.h>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

ServerThread::ServerThread() {

}
ServerThread::~ServerThread() {
    SAFE_DELETE_MAP(m_Clients);
}
unsigned int ServerThread::num_clients() const {
    return static_cast<unsigned int>(m_Clients.size());
}
unordered_map<string, Engine::Networking::ServerClient*>& ServerThread::clients() const {
    return m_Clients;
}
bool ServerThread::remove_client(const std::string& hash, Engine::Networking::Server& server) {
    bool has_client_hash        = m_Clients.count(hash);
    bool has_server_client_hash = server.m_HashedClients.count(hash);
    if (has_client_hash && has_server_client_hash) {
        auto* client = m_Clients.at(hash);
        SAFE_DELETE(client);
        m_Clients.erase(hash);
        server.m_HashedClients.erase(hash);
        return true;
    }else {
        if (!has_client_hash) {
            std::cout << "error: client removal - hash: " << hash << " is not in m_Clients\n";
        }
        if (!has_server_client_hash) {
            std::cout << "error: client removal - hash: " << hash << " is not in server.m_HashedClients\n";
        }
        return false;
    }
    return false;
}
bool ServerThread::add_client(string& hash, Engine::Networking::ServerClient* client, Engine::Networking::Server& server) {
    bool has_client_hash        = m_Clients.count(hash);
    bool has_server_client_hash = server.m_HashedClients.count(hash);
    if (!has_client_hash && !has_server_client_hash) {
        m_Clients.emplace(hash, client);
        server.m_HashedClients.emplace(hash, client);
        return true;
    }else{
        if (has_client_hash) {
            std::cout << "error: client addition - hash: " << hash << " is already in m_Clients\n";
        }
        if (has_server_client_hash) {
            std::cout << "error: client addition - hash: " << hash << " is already in server.m_HashedClients\n";
        }
        return false;
    }
    return false;
}
ServerThread::ServerThread(ServerThread&& other) noexcept {
    m_Clients = std::move(other.m_Clients);
}
ServerThread& ServerThread::operator=(ServerThread&& other) noexcept {
    if (&other != this) {
        m_Clients = std::move(other.m_Clients);
    }
    return *this;
}
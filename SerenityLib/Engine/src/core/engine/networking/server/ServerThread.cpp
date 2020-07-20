#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/networking/server/ServerThread.h>
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/server/Server.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/threading/ThreadingModule.h>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

ServerThread::ServerThread() {
}
ServerThread::~ServerThread() {
    SAFE_DELETE_MAP(m_ServerClients);
}
bool ServerThread::remove_client(const string& hash, Server& server) {
    bool has_client_hash        = m_ServerClients.count(hash);
    bool has_server_client_hash = server.m_HashedClients.count(hash);
    if (has_client_hash && has_server_client_hash) {
        auto* serverClient = m_ServerClients.at(hash);
        SAFE_DELETE(serverClient);
        m_ServerClients.erase(hash);
        server.m_HashedClients.erase(hash);
        return true;
    }
    if (!has_client_hash) {
        std::cout << "error: client removal - hash: " << hash << " is not in m_Clients\n";
    }
    if (!has_server_client_hash) {
        std::cout << "error: client removal - hash: " << hash << " is not in server.m_HashedClients\n";
    }
    return false;
}
bool ServerThread::add_client(const string& hash, ServerClient* serverClient, Server& server) {
    bool has_client_hash        = m_ServerClients.count(hash);
    bool has_server_client_hash = server.m_HashedClients.count(hash);
    if (!has_client_hash && !has_server_client_hash) {
        m_ServerClients.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(hash), 
            std::forward_as_tuple(serverClient)
        );
        server.m_HashedClients.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(hash),
            std::forward_as_tuple(serverClient)
        );
        return true;
    }
    if (has_client_hash) {
        std::cout << "error: client addition - hash: " << hash << " is already in m_Clients\n";
    }
    if (has_server_client_hash) {
        std::cout << "error: client addition - hash: " << hash << " is already in server.m_HashedClients\n";
    }
    return false;
}

ServerThread::ServerThread(ServerThread&& other) noexcept {
    m_ServerClients = std::move(other.m_ServerClients);
}
ServerThread& ServerThread::operator=(ServerThread&& other) noexcept {
    if (&other != this) {
        m_ServerClients = std::move(other.m_ServerClients);
    }
    return *this;
}






ServerThreadCollection::ServerThreadCollection(size_t threadCount) {
    m_Threads.resize(threadCount);
}
ServerThreadCollection::~ServerThreadCollection() {

}
void ServerThreadCollection::setBlocking(bool blocking) {
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].m_ServerClients) {
            client.second->socket()->setBlocking(blocking);
        }
    }
}
void ServerThreadCollection::setBlocking(const string& hash, bool blocking) {
    for (auto& thread : m_Threads) {
        if (thread.m_ServerClients.count(hash)) {
            thread.m_ServerClients.at(hash)->socket()->setBlocking(blocking);
            return;
        }
    }
}
bool ServerThreadCollection::addClient(const string& hash, ServerClient* client, Server& server) {
    auto next_thread = getNextAvailableClientThread();
    if (next_thread) {
        bool result = next_thread->add_client(hash, client, server);
        if (result) {
            ++m_NumClients;
        }
        return result;
    }
#ifndef ENGINE_PRODUCTION
    else {
        std::cout << "ServerThreadCollection::addClient() could not get a next thread\n";
    }
#endif
    return false;
}
bool ServerThreadCollection::removeClient(const string& hash, Server& server) {
    bool complete = false;
    bool result   = false;
    for (auto& thread : m_Threads) {
        for (auto& client_itr : thread.m_ServerClients) {
            if (client_itr.first == hash) {
                result   = thread.remove_client(hash, server);
                if (result) {
                    --m_NumClients;
                }
                complete = true;
                break;
            }
        }
        if (complete) {
            break;
        }
    }
    return result;
}
ServerThread* ServerThreadCollection::getNextAvailableClientThread() {
    Engine::Networking::ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = &clientThread;
        }
    }
    return leastThread;
}

#include <serenity/core/engine/networking/server/ServerThread.h>
#include <serenity/core/engine/networking/server/ServerClient.h>
#include <serenity/core/engine/networking/SocketTCP.h>

using namespace Engine;
using namespace Engine::Networking;

bool ServerThread::remove_client(const std::string& hash) {
    bool has_client_hash        = m_HashedServerClients.contains(hash);
    if (has_client_hash) {
        m_HashedServerClients.erase(hash);
        return true;
    }
    if (!has_client_hash) {
        ENGINE_PRODUCTION_LOG("error: ServerThread::remove_client() client removal - hash: " << hash << " is not in m_Clients")
    }
    return false;
}
bool ServerThread::add_client(const std::string& hash, ServerClient* serverClient) {
    bool has_client_hash = m_HashedServerClients.contains(hash);
    if (!has_client_hash) {
        m_HashedServerClients.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(hash), 
            std::forward_as_tuple(std::unique_ptr<ServerClient>(serverClient))
        );
        return true;
    }
    if (has_client_hash) {
        ENGINE_PRODUCTION_LOG("error: ServerThread::add_client() client addition - hash: " << hash << " is already in m_Clients")
    }
    return false;
}
void ServerThread::clearAllClients() {
    m_HashedServerClients.clear();
}
ServerThread::ServerThread(ServerThread&& other) noexcept {
    m_HashedServerClients = std::move(other.m_HashedServerClients);
}
ServerThread& ServerThread::operator=(ServerThread&& other) noexcept {
    if (&other != this) {
        m_HashedServerClients = std::move(other.m_HashedServerClients);
    }
    return *this;
}






ServerThreadContainer::ServerThreadContainer(size_t threadCount) {
    m_Threads.resize(threadCount);
}
void ServerThreadContainer::setBlocking(bool blocking) {
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].m_HashedServerClients) {
            client.second->socket()->setBlocking(blocking);
        }
    }
}
void ServerThreadContainer::setBlocking(const std::string& hash, bool blocking) {
    for (auto& thread : m_Threads) {
        if (thread.m_HashedServerClients.contains(hash)) {
            thread.m_HashedServerClients.at(hash)->socket()->setBlocking(blocking);
            return;
        }
    }
}
bool ServerThreadContainer::addClient(const std::string& hash, ServerClient* client) {
    auto next_thread = getNextAvailableClientThread();
    if (next_thread) {
        bool result = next_thread->add_client(hash, client);
        if (result) {
            ++m_NumClients;
        }
        return result;
    }else{
        ENGINE_PRODUCTION_LOG("ServerThreadCollection::addClient() could not get a next thread")
    }
    return false;
}
void ServerThreadContainer::removeAllClients() {
    for (auto& thread : m_Threads) {
        thread.clearAllClients();
    }
    m_NumClients = 0;
}
bool ServerThreadContainer::removeClient(const std::string& hash) {
    bool complete = false;
    bool result   = false;
    for (auto& thread : m_Threads) {
        for (auto&[name, client] : thread.m_HashedServerClients) {
            if (name == hash) {
                result   = thread.remove_client(hash);
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
ServerThread* ServerThreadContainer::getNextAvailableClientThread() {
    Engine::Networking::ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = &clientThread;
        }
    }
    return leastThread;
}
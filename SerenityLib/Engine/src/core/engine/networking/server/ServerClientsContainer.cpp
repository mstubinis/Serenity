#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/server/ServerClientsContainer.h>
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/threading/ThreadingModule.h>

using namespace Engine::Networking;

ServerClientsContainer::ServerClientsContainer(size_t numThreads) 
    : m_ServerClientThreadsCollection{ numThreads }
{
    m_HashedClients.reserve(numThreads);
    m_RemovedClients.reserve(numThreads);
}
ServerClient* ServerClientsContainer::get(const std::string& hash) const noexcept {
    return (m_HashedClients.count(hash)) ? m_HashedClients.at(hash) : nullptr;
}
bool ServerClientsContainer::remove_client_delayed(ServerClient& inClient, std::mutex& inMutex) noexcept {
    for (const auto& [name, client] : m_HashedClients) {
        if (client == &inClient) {
            inClient.disconnect();
            {
                std::lock_guard lock(inMutex);
                m_RemovedClients.emplace_back(
                    std::piecewise_construct,
                    std::forward_as_tuple(name),
                    std::forward_as_tuple(client)
                );
            }
            ENGINE_PRODUCTION_LOG("ServerClientsContainer::remove_client_delayed - ip: " << client->ip() << ", port: " << client->port() << " - has been completely removed from the server")
            return true;
        }
    }
    return false;
}
bool ServerClientsContainer::remove_client(const std::string& hash) noexcept {
    bool has_client_hash        = m_ServerClientThreadsCollection.has(hash);
    bool has_server_client_hash = m_HashedClients.count(hash);
    if (has_client_hash && has_server_client_hash) {
        m_HashedClients.erase(hash);
        m_ServerClientThreadsCollection.removeClient(hash);
        return true;
    }
    if (!has_client_hash) {
        ENGINE_PRODUCTION_LOG("error: ServerClientsContainer::remove_client() client removal - hash: " << hash << " is not in m_Clients")
    }
    if (!has_server_client_hash) {
        ENGINE_PRODUCTION_LOG("error: ServerClientsContainer::remove_client() client removal - hash: " << hash << " is not in server.m_HashedClients")
    }
    return false;
}
bool ServerClientsContainer::add_client(const std::string& hash, ServerClient* serverClient) {
    bool has_client_hash        = m_ServerClientThreadsCollection.has(hash);
    bool has_server_client_hash = m_HashedClients.count(hash);
    if (!has_client_hash && !has_server_client_hash) {
        auto* nextThread = m_ServerClientThreadsCollection.getNextAvailableClientThread();
        if (!nextThread) {
            ENGINE_PRODUCTION_LOG("error: ServerClientsContainer::add_client() could not find a nextThread!")
            return false;
        }
        nextThread->add_client(hash, serverClient);
        m_HashedClients.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(hash),
            std::forward_as_tuple(serverClient)
        );
        return true;
    }
    if (has_client_hash) {
        ENGINE_PRODUCTION_LOG("error: ServerClientsContainer::add_client() client addition - hash: " << hash << " is already in m_Clients")
    }
    if (has_server_client_hash) {
        ENGINE_PRODUCTION_LOG("error: ServerClientsContainer::add_client() client addition - hash: " << hash << " is already in server.m_HashedClients")
    }
    return false;
}
void ServerClientsContainer::internal_update_remove_clients() noexcept {
    if (m_RemovedClients.size() > 0) {
        Engine::priv::threading::waitForAll();
        for (const auto& [hash, client] : m_RemovedClients) {
            bool result = m_ServerClientThreadsCollection.removeClient(hash);
        }
        m_RemovedClients.clear();
    }
}
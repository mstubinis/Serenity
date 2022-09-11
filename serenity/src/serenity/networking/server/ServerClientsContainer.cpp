
#include <serenity/networking/server/ServerClientsContainer.h>
#include <serenity/networking/server/ServerClient.h>
#include <serenity/threading/ThreadingModule.h>

Engine::Networking::ServerClientsContainer::ServerClientsContainer(size_t numThreads)
    : m_ThreadContainer{ numThreads }
{}
bool Engine::Networking::ServerClientsContainer::addClient(std::string_view hash, ServerClient* serverClient) {
    bool result = m_ThreadContainer.addClient(hash, serverClient);
    if (result) {
        m_HashedClients.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(serverClient));
        return result;
    }
    ENGINE_LOG(__FUNCTION__ << "(): failed to add client!") 
    return false;
}
bool Engine::Networking::ServerClientsContainer::removeClientImmediately(ServerClient& inClient, std::mutex& inMutex) {
    std::string foundHash;
    bool removed = false;
    for (const auto& [name, client] : m_HashedClients) {
        if (client == &inClient) {
            inClient.disconnect();
            foundHash = name;
            break;
        }
    }
    if (!foundHash.empty()) {
        {
            std::scoped_lock lock{ inMutex };
            Engine::erase(m_HashedClients, m_HashedClients.find(foundHash));
            removed = m_ThreadContainer.removeClient(foundHash);    
        }
    }
    if (!removed) {
        ENGINE_LOG(__FUNCTION__ << "(): could not remove client hash: " << foundHash)
    }
    return removed;
}
bool Engine::Networking::ServerClientsContainer::removeClient(ServerClient& inClient, std::mutex& inMutex) {
    for (const auto& [name, client] : m_HashedClients) {
        if (client == &inClient) {
            inClient.disconnect();
            {
                std::scoped_lock lock{ inMutex };
                m_RemovedClients.emplace_back(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(client));
            }
            ENGINE_LOG(__FUNCTION__ << "(): ip: " << client->ip() << ", port: " << client->port() << " - has been completely removed from the server")
            return true;
        }
    }
    return false;
}
void Engine::Networking::ServerClientsContainer::clearAllClients() {
    m_HashedClients.clear();
    m_ThreadContainer.removeAllClients();
    m_RemovedClients.clear();
}
void Engine::Networking::ServerClientsContainer::internal_update_remove_clients() noexcept {
    if (m_RemovedClients.size() > 0) {
        Engine::priv::threading::waitForAll();
        for (const auto& [hash, client] : m_RemovedClients) {
            Engine::erase(m_HashedClients, m_HashedClients.find(hash));
            /*bool result = */m_ThreadContainer.removeClient(hash);
        }
        m_RemovedClients.clear();
    }
}
#include <serenity/core/engine/networking/server/ServerThreadCollection.h>
#include <serenity/core/engine/networking/server/ServerClient.h>
#include <serenity/core/engine/networking/SocketTCP.h>

Engine::Networking::ServerThreadCollection::ServerThreadCollection(size_t threadCount) {
    m_ServerClientThreads.resize(threadCount);
}
void Engine::Networking::ServerThreadCollection::setBlocking(bool blocking) {
    for (auto& thread : m_ServerClientThreads) {
        for (const auto& [hash, client] : thread) {
            client->socket()->setBlocking(blocking);
        }
    }
}
void Engine::Networking::ServerThreadCollection::setBlocking(const std::string& hash, bool blocking) {
    for (auto& thread : m_ServerClientThreads) {
        if (thread.m_HashedServerClients.count(hash)) {
            thread.m_HashedServerClients.at(hash)->socket()->setBlocking(blocking);
            return;
        }
    }
}
bool Engine::Networking::ServerThreadCollection::addClient(const std::string& hash, ServerClient* client) {
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
bool Engine::Networking::ServerThreadCollection::removeClient(const std::string& hash) {
    bool complete = false;
    bool result = false;
    for (auto& thread : m_ServerClientThreads) {
        for (const auto& [name, client] : thread) {
            if (name == hash) {
                result = thread.remove_client(hash);
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
Engine::Networking::ServerThread* Engine::Networking::ServerThreadCollection::getNextAvailableClientThread() {
    Engine::Networking::ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_ServerClientThreads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = &clientThread;
        }
    }
    return leastThread;
}
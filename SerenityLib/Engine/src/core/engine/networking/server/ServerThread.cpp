#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/server/ServerThread.h>

using namespace Engine;
using namespace Engine::Networking;

ServerThread::~ServerThread() {
    SAFE_DELETE_MAP(m_HashedServerClients);
}
bool ServerThread::remove_client(const std::string& hash) {
    if (m_HashedServerClients.count(hash)) {
        m_HashedServerClients.erase(hash);
        return true;
    }
    return false;
}
bool ServerThread::add_client(const std::string& hash, ServerClient* serverClient) {
    if (!m_HashedServerClients.count(hash)) {
        m_HashedServerClients.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(hash), 
            std::forward_as_tuple(serverClient)
        );
        return true;
    }
    return false;
}
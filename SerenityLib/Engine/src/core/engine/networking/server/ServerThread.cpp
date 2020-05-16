#include <core/engine/networking/server/ServerThread.h>
#include <core/engine/networking/server/ServerClient.h>

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
bool ServerThread::add_client(string& hash, Engine::Networking::ServerClient* client) {
    if (m_Clients.count(hash)) {
        return false;
    }
    m_Clients.emplace(hash, client);
    return true;
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
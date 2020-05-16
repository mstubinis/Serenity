#include <core/engine/networking/SocketManager.h>
#include <core/engine/networking/Networking.h>

using namespace std;
using namespace Engine::priv;

SocketManager::SocketManager() {

}
SocketManager::~SocketManager() {

}

void SocketManager::update(const float dt) {
    for (size_t i = 0; i < m_TCPListeners.size(); ++i) {
        m_TCPListeners[i]->update(dt);
    }
    for (size_t i = 0; i < m_TCPSockets.size(); ++i) {
        m_TCPSockets[i]->update(dt);
    }
    for (size_t i = 0; i < m_UDPSockets.size(); ++i) {
        m_UDPSockets[i]->update(dt);
    }
}

void SocketManager::add_tcp_socket(Engine::Networking::SocketTCP* socketTCP) {
    internal_add_socket(m_TCPSockets, socketTCP);
}
void SocketManager::add_udp_socket(Engine::Networking::SocketUDP* socketUDP) {
    internal_add_socket(m_UDPSockets, socketUDP);
}
void SocketManager::add_tcp_listener(Engine::Networking::ListenerTCP* listenerTCP) {
    internal_add_socket(m_TCPListeners, listenerTCP);
}

void SocketManager::remove_tcp_socket(Engine::Networking::SocketTCP* socketTCP) {
    internal_remove_socket(m_TCPSockets, socketTCP);
}
void SocketManager::remove_udp_socket(Engine::Networking::SocketUDP* socketUDP) {
    internal_remove_socket(m_UDPSockets, socketUDP);
}
void SocketManager::remove_tcp_listener(Engine::Networking::ListenerTCP* listenerTCP) {
    internal_remove_socket(m_TCPListeners, listenerTCP);
}
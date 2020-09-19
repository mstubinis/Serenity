#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/SocketManager.h>
#include <core/engine/networking/Networking.h>

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
void SocketManager::add_tcp_socket(Engine::Networking::SocketTCP* socketTCPPtr) {
    internal_add_socket(m_TCPSockets, socketTCPPtr);
}
void SocketManager::add_udp_socket(Engine::Networking::SocketUDP* socketUDPPtr) {
    internal_add_socket(m_UDPSockets, socketUDPPtr);
}
void SocketManager::add_tcp_listener(Engine::Networking::ListenerTCP* listenerTCPPtr) {
    internal_add_socket(m_TCPListeners, listenerTCPPtr);
}
void SocketManager::remove_tcp_socket(Engine::Networking::SocketTCP* socketTCPPtr) {
    internal_remove_socket(m_TCPSockets, socketTCPPtr);
}
void SocketManager::remove_udp_socket(Engine::Networking::SocketUDP* socketUDPPtr) {
    internal_remove_socket(m_UDPSockets, socketUDPPtr);
}
void SocketManager::remove_tcp_listener(Engine::Networking::ListenerTCP* listenerTCPPtr) {
    internal_remove_socket(m_TCPListeners, listenerTCPPtr);
}
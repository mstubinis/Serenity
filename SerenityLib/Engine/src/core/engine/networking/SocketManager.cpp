#include <core/engine/networking/SocketManager.h>
#include <core/engine/networking/Networking.h>

using namespace std;
using namespace Engine::priv;

SocketManager::SocketManager() {

}
SocketManager::~SocketManager() {

}

void SocketManager::update(const float dt) {
    for (int i = 0; i < m_TCPListeners.size(); ++i) {
        m_TCPListeners[i]->update(dt);
    }
    for (int i = 0; i < m_TCPSockets.size(); ++i) {
        m_TCPSockets[i]->update(dt);
    }
    for (int i = 0; i < m_UDPSockets.size(); ++i) {
        m_UDPSockets[i]->update(dt);
    }
}

void SocketManager::add_tcp_socket(Engine::Networking::SocketTCP* socketTCP) {
    for (int i = 0; i < m_TCPSockets.size(); ++i) {
        if (m_TCPSockets[i] == socketTCP) {
            return;
        }
    }
    m_TCPSockets.push_back(socketTCP);
}
void SocketManager::add_udp_socket(Engine::Networking::SocketUDP* socketUDP) {
    for (int i = 0; i < m_UDPSockets.size(); ++i) {
        if (m_UDPSockets[i] == socketUDP) {
            return;
        }
    }
    m_UDPSockets.push_back(socketUDP);
}
void SocketManager::add_tcp_listener(Engine::Networking::ListenerTCP* listenerTCP) {
    for (int i = 0; i < m_TCPListeners.size(); ++i) {
        if (m_TCPListeners[i] == listenerTCP) {
            return;
        }
    }
    m_TCPListeners.push_back(listenerTCP);
}

void SocketManager::remove_tcp_socket(Engine::Networking::SocketTCP* socketTCP) {
    for (int i = 0; i < m_TCPSockets.size(); ++i) {
        if (m_TCPSockets[i] == socketTCP) {
            m_TCPSockets.erase(m_TCPSockets.begin() + i);
            return;
        }
    }
}
void SocketManager::remove_udp_socket(Engine::Networking::SocketUDP* socketUDP) {
    for (int i = 0; i < m_UDPSockets.size(); ++i) {
        if (m_UDPSockets[i] == socketUDP) {
            m_UDPSockets.erase(m_UDPSockets.begin() + i);
            return;
        }
    }
}
void SocketManager::remove_tcp_listener(Engine::Networking::ListenerTCP* listenerTCP) {
    for (int i = 0; i < m_TCPListeners.size(); ++i) {
        if (m_TCPListeners[i] == listenerTCP) {
            m_TCPListeners.erase(m_TCPListeners.begin() + i);
            return;
        }
    }
}
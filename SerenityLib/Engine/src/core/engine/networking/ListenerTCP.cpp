#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/events/EventModule.h>
#include <core/engine/events/EventIncludes.h>
#include <core/engine/events/Event.h>
#include <core/engine/system/Engine.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

Networking::ListenerTCP::ListenerTCP(const unsigned short port, const string& ip){
    m_Port = port;
    m_IP   = ip;
    m_Listener.setBlocking(false);

    Core::m_Engine->m_NetworkingModule.m_SocketManager.add_tcp_listener(this);
}
Networking::ListenerTCP::~ListenerTCP() { 
    Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_tcp_listener(this);
    close(); 
}
void Networking::ListenerTCP::update(const float dt) {

}
unsigned short Networking::ListenerTCP::localPort() const {
    return m_Listener.getLocalPort(); 
}
void Networking::ListenerTCP::setBlocking(bool b) { 
    m_Listener.setBlocking(b); 
}
bool Networking::ListenerTCP::isListening() const {
    return (localPort() != 0);
}
bool Networking::ListenerTCP::isBlocking() const {
    return m_Listener.isBlocking(); 
}
void Networking::ListenerTCP::close() { 
    if (isListening()) {
        EventSocket e = EventSocket(m_Listener.getLocalPort(), 0, m_IP, SocketType::TCPListener);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);
        m_Listener.close();
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Networking::ListenerTCP::accept(sf::TcpSocket& sfSocketTCP) {
    return SocketStatus::map_status(m_Listener.accept(sfSocketTCP));
}
SocketStatus::Status Networking::ListenerTCP::accept(SocketTCP& socketTCP) {
    return SocketStatus::map_status(m_Listener.accept(socketTCP.m_SocketTCP));
}
SocketStatus::Status Networking::ListenerTCP::listen() {
    m_IP = (m_IP.empty() || m_IP == "0.0.0.0") ? "0.0.0.0" : m_IP;
    auto status = m_Listener.listen(m_Port, m_IP);
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_Listener.getLocalPort(), 0, m_IP, SocketType::TCPListener);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
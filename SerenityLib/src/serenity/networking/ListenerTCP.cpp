
#include <serenity/networking/ListenerTCP.h>
#include <serenity/networking/SocketTCP.h>
#include <serenity/events/EventModule.h>
#include <serenity/events/EventIncludes.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>
#include <serenity/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;

Networking::ListenerTCP::ListenerTCP(const uint16_t port, const std::string& ip)
    : m_Port{ port }
    , m_IP{ ip }
{
    m_Listener.setBlocking(false);

    Core::m_Engine->m_NetworkingModule.m_SocketManager.add_tcp_listener(this);
}
Networking::ListenerTCP::~ListenerTCP() { 
    Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_tcp_listener(this);
    close(); 
}
void Networking::ListenerTCP::update(const float dt) {

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
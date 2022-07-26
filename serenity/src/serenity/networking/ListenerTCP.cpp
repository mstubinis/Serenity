
#include <serenity/networking/ListenerTCP.h>
#include <serenity/networking/SocketTCP.h>
#include <serenity/events/EventModule.h>
#include <serenity/events/EventIncludes.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>
#include <serenity/utils/Utils.h>


Engine::Networking::ListenerTCP::ListenerTCP(const uint16_t port, const std::string& ip)
    : m_IP{ ip }
    , m_Port{ port }
{
    m_Listener.setBlocking(false);

    Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.add_tcp_listener(this);
}
Engine::Networking::ListenerTCP::~ListenerTCP() { 
    Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_tcp_listener(this);
    close(); 
}
void Engine::Networking::ListenerTCP::update(const float dt) {

}
void Engine::Networking::ListenerTCP::close() { 
    if (isListening()) {
        Engine::priv::EventSocket e = Engine::priv::EventSocket(m_Listener.getLocalPort(), 0, m_IP, SocketType::TCPListener);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);
        m_Listener.close();
        Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Engine::Networking::ListenerTCP::accept(sf::TcpSocket& sfSocketTCP) {
    return SocketStatus::map_status(m_Listener.accept(sfSocketTCP));
}
SocketStatus::Status Engine::Networking::ListenerTCP::accept(SocketTCP& socketTCP) {
    return SocketStatus::map_status(m_Listener.accept(socketTCP.m_SocketTCP));
}
SocketStatus::Status Engine::Networking::ListenerTCP::listen() {
    m_IP = (m_IP.empty() || m_IP == "0.0.0.0") ? "0.0.0.0" : m_IP;
    auto status = m_Listener.listen(m_Port, m_IP);
    if (status == sf::Socket::Status::Done) {
        Engine::priv::EventSocket e = Engine::priv::EventSocket(m_Listener.getLocalPort(), 0, m_IP, SocketType::TCPListener);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
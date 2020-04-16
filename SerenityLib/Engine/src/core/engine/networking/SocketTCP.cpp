#include <core/engine/networking/SocketTCP.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/events/Engine_EventIncludes.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/system/Engine.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

Networking::SocketTCP::SocketTCP(){
    Core::m_Engine->m_Misc.m_SocketManager.add_tcp_socket(this);
}
Networking::SocketTCP::SocketTCP(const unsigned short port, const string& ip) : Networking::SocketTCP::SocketTCP() {  //client side socket
    m_IP    = ip;
    m_Port  = port;
}
Networking::SocketTCP::~SocketTCP() { 
    Core::m_Engine->m_Misc.m_SocketManager.remove_tcp_socket(this);
    disconnect();
}

const sf::Socket::Status Networking::SocketTCP::internal_send_packet(sf::Packet& packet) {
    const auto status = m_Socket.send(packet);
    switch (status) {
        case sf::Socket::Status::Done: {
            EventPacket e(&packet);
            Event ev(EventType::PacketSent);
            ev.eventPacket = e;
            Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
            break;
        }
        case sf::Socket::Status::Disconnected: {
            break;
        }
        case sf::Socket::Status::Error: {
            break;
        }
        case sf::Socket::Status::NotReady: {
            break;
        }
        case sf::Socket::Status::Partial: {
            break;
        }
        default: {
            break;
        }
    }
    return status;
}

void Networking::SocketTCP::update(const float dt) {
    if (!m_PartialPackets.empty()) {
        const auto status = internal_send_packet(m_PartialPackets.front());
        if (status == sf::Socket::Status::Done) {
            m_PartialPackets.pop();
        }
    }
}
const bool Networking::SocketTCP::isConnected() const {
    return (m_Socket.getLocalPort() != 0);
}
sf::TcpSocket& Networking::SocketTCP::socket() {
    return m_Socket;
}
const string Networking::SocketTCP::ip() const {
    return m_Socket.getRemoteAddress().toString();
}
const unsigned short Networking::SocketTCP::remotePort() const {
    return m_Socket.getRemotePort();
}
const unsigned short Networking::SocketTCP::localPort() const {
    return m_Socket.getLocalPort();
}
void Networking::SocketTCP::setBlocking(const bool blocking) {
    m_Socket.setBlocking(blocking);
}
const bool Networking::SocketTCP::isBlocking() const {
    return m_Socket.isBlocking();
}
void Networking::SocketTCP::disconnect() {
    if (isConnected()) {
        EventSocket e = EventSocket(m_Socket.getLocalPort(), m_Socket.getRemotePort(), m_Socket.getRemoteAddress(), SocketType::TCP);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);

        m_Socket.disconnect();

        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    //TODO: is this needed outside the if block?
    //m_Socket.disconnect();
}
const sf::Socket::Status Networking::SocketTCP::connect(const unsigned short timeout) {
    const auto status = m_Socket.connect(m_IP, m_Port, sf::seconds(timeout));
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_Socket.getLocalPort(), m_Socket.getRemotePort(), m_Socket.getRemoteAddress(), SocketType::TCP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
const sf::Socket::Status Networking::SocketTCP::send(sf::Packet& packet) {
    m_PartialPackets.push(packet);
    auto& front       = m_PartialPackets.front();
    const auto status = internal_send_packet(front);
    if (status == sf::Socket::Status::Done) {
        m_PartialPackets.pop();
    }
    return status;
}
const sf::Socket::Status Networking::SocketTCP::send(const void* data, size_t size) {
    return m_Socket.send(data, size);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* data, size_t size, size_t sent) {
    return m_Socket.send(data, size, sent);
}
const sf::Socket::Status Networking::SocketTCP::receive(sf::Packet& packet) {
    const auto status = m_Socket.receive(packet);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
const sf::Socket::Status Networking::SocketTCP::receive(void* data, size_t size, size_t sent) {
    return m_Socket.receive(data, size, sent);
}
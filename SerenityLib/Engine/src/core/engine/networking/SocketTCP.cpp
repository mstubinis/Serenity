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

sf::Socket::Status Networking::SocketTCP::internal_send_packet(sf::Packet& packet) {
    auto status = m_SocketTCP.send(packet);
    switch (status) {
        case sf::Socket::Status::Done: {
            EventPacket e(&packet);
            Event ev(EventType::PacketSent);
            ev.eventPacket = e;
            Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
            break;
        }case sf::Socket::Status::Disconnected: {
            break;
        }case sf::Socket::Status::Error: {
            break;
        }case sf::Socket::Status::NotReady: {
            break;
        }case sf::Socket::Status::Partial: {
            break;
        }default: {
            break;
        }
    }
    return status;
}
sf::Socket::Status Networking::SocketTCP::internal_send_partial_packets_loop() {
    sf::Socket::Status status = sf::Socket::Status::Error;
    if (!m_PartialPackets.empty()) {
        status = internal_send_packet(m_PartialPackets.front());
        switch (status) {
            case sf::Socket::Status::Done: {
                m_PartialPackets.pop();
                break;
            }case sf::Socket::Status::Disconnected: {
                break;
            }case sf::Socket::Status::Error: {
                break;
            }case sf::Socket::Status::NotReady: {
                break;
            }case sf::Socket::Status::Partial: {
                break;
            }default: {
                break;
            }
        }
    }
    return status;
}
void Networking::SocketTCP::update(const float dt) {
    if (isConnected()) {
        auto status = internal_send_partial_packets_loop();
    }
}
bool Networking::SocketTCP::isConnected() const {
    return (m_SocketTCP.getLocalPort() != 0);
}
sf::TcpSocket& Networking::SocketTCP::socket() {
    return m_SocketTCP;
}
string Networking::SocketTCP::ip() const {
    return m_SocketTCP.getRemoteAddress().toString();
}
unsigned short Networking::SocketTCP::remotePort() const {
    return m_SocketTCP.getRemotePort();
}
unsigned short Networking::SocketTCP::localPort() const {
    return m_SocketTCP.getLocalPort();
}
void Networking::SocketTCP::setBlocking(const bool blocking) {
    m_SocketTCP.setBlocking(blocking);
}
bool Networking::SocketTCP::isBlocking() const {
    return m_SocketTCP.isBlocking();
}
void Networking::SocketTCP::disconnect() {
    if (isConnected()) {
        EventSocket e = EventSocket(m_SocketTCP.getLocalPort(), m_SocketTCP.getRemotePort(), m_SocketTCP.getRemoteAddress(), SocketType::TCP);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);

        m_SocketTCP.disconnect();

        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    //TODO: is this needed outside the if block?
    //m_Socket.disconnect();
}
sf::Socket::Status Networking::SocketTCP::connect(const unsigned short timeout) {
    auto status = m_SocketTCP.connect(m_IP, m_Port, sf::seconds(timeout));
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_SocketTCP.getLocalPort(), m_SocketTCP.getRemotePort(), m_SocketTCP.getRemoteAddress(), SocketType::TCP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
sf::Socket::Status Networking::SocketTCP::send(Engine::Networking::Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    return send(sf_packet);
}
sf::Socket::Status Networking::SocketTCP::send(sf::Packet& packet) {
    m_PartialPackets.push(packet);
    auto status = internal_send_partial_packets_loop();
    return status;
}
sf::Socket::Status Networking::SocketTCP::send(const void* data, size_t size) {
    return m_SocketTCP.send(data, size);
}
sf::Socket::Status Networking::SocketTCP::send(const void* data, size_t size, size_t& sent) {
    return m_SocketTCP.send(data, size, sent);
}
sf::Socket::Status Networking::SocketTCP::receive(sf::Packet& packet) {
    auto status = m_SocketTCP.receive(packet);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
sf::Socket::Status Networking::SocketTCP::receive(void* data, size_t size, size_t& received) {
    return m_SocketTCP.receive(data, size, received);
}
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/events/EventModule.h>
#include <core/engine/events/EventIncludes.h>
#include <core/engine/events/Event.h>
#include <core/engine/system/Engine.h>
#include <core/engine/utils/Utils.h>

#include <chrono>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

Networking::SocketTCP::SocketTCP(){
    Core::m_Engine->m_NetworkingModule.m_SocketManager.add_tcp_socket(this);
}
Networking::SocketTCP::SocketTCP(const unsigned short port, const string& ip) : Networking::SocketTCP::SocketTCP() {  //client side socket
    m_IP    = ip;
    m_Port  = port;
}
Networking::SocketTCP::~SocketTCP() { 
    Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_tcp_socket(this);
    disconnect();
}

SocketStatus::Status Networking::SocketTCP::internal_send_packet(sf::Packet& packet) {
    auto status = m_SocketTCP.send(packet);
    switch (status) {
        case sf::Socket::Status::Done: {
            EventPacket e(&packet);
            Event ev(EventType::PacketSent);
            ev.eventPacket = e;
            Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
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
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketTCP::internal_send_partial_packets_loop() {
    SocketStatus::Status status = SocketStatus::Error;
    if (!m_PartialPackets.empty()) {
        status = internal_send_packet(m_PartialPackets.front());
        switch (status) {
            case SocketStatus::Done: {
                m_PartialPackets.pop();
                break;
            }case SocketStatus::Disconnected: {
                break;
            }case SocketStatus::Error: {
                break;
            }case SocketStatus::NotReady: {
                break;
            }case SocketStatus::Partial: {
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

        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Networking::SocketTCP::connect(const unsigned short timeout) {
    auto status = m_SocketTCP.connect(m_IP, m_Port, sf::seconds(timeout));
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_SocketTCP.getLocalPort(), m_SocketTCP.getRemotePort(), m_SocketTCP.getRemoteAddress(), SocketType::TCP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketTCP::send(Engine::Networking::Packet& packet) {
    sf::Packet sf_packet;
    using cast         = std::chrono::duration<std::uint64_t>;
    packet.m_Timestamp = std::chrono::duration_cast<cast>(std::chrono::system_clock::now().time_since_epoch()).count();
    packet.build(sf_packet);
    return send(sf_packet);
}
SocketStatus::Status Networking::SocketTCP::send(sf::Packet& packet) {
    m_PartialPackets.push(packet);
    return internal_send_partial_packets_loop();
}
SocketStatus::Status Networking::SocketTCP::send(const void* data, size_t size) {
    return SocketStatus::map_status(m_SocketTCP.send(data, size));
}
SocketStatus::Status Networking::SocketTCP::send(const void* data, size_t size, size_t& sent) {
    return SocketStatus::map_status(m_SocketTCP.send(data, size, sent));
}
SocketStatus::Status Networking::SocketTCP::receive(sf::Packet& packet) {
    auto status = m_SocketTCP.receive(packet);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketTCP::receive(void* data, size_t size, size_t& received) {
    return SocketStatus::map_status(m_SocketTCP.receive(data, size, received));
}
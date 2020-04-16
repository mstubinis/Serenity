#include <core/engine/networking/SocketUDP.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/events/Engine_EventIncludes.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/system/Engine.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

Networking::SocketUDP::SocketUDP(const unsigned short port, const string& ip){
    m_Port  = port;
    m_IP    = ip;

    Core::m_Engine->m_Misc.m_SocketManager.add_udp_socket(this);
}
Networking::SocketUDP::~SocketUDP() { 
    Core::m_Engine->m_Misc.m_SocketManager.remove_udp_socket(this);
}

const sf::Socket::Status Networking::SocketUDP::internal_send_packet(UDPPacketInfo& packet) {
    const auto status = m_Socket.send(packet.packet, packet.ip, packet.port);
    switch (status) {
        case sf::Socket::Status::Done: {
            EventPacket e(&packet.packet);
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

void Networking::SocketUDP::update(const float dt) {
    if (!m_PartialPackets.empty()) {
        const auto status = internal_send_packet(m_PartialPackets.front());
        if (status == sf::Socket::Status::Done) {
            m_PartialPackets.pop();
        }
    }
}
sf::IpAddress Networking::SocketUDP::internal_ip(const string& ip) const {
    return (ip.empty()) ? sf::IpAddress::Any : ip;
}
const bool Networking::SocketUDP::isBound() const {
    return (m_Socket.getLocalPort() != 0);
}
sf::UdpSocket& Networking::SocketUDP::socket() {
    return m_Socket; 
}
const unsigned short Networking::SocketUDP::localPort() const {
    return m_Socket.getLocalPort(); 
}
void Networking::SocketUDP::setBlocking(bool b) { 
    m_Socket.setBlocking(b); 
}
const bool Networking::SocketUDP::isBlocking() const {
    return m_Socket.isBlocking(); 
}
void Networking::SocketUDP::changePort(const unsigned short newPort) {
    const bool was_bound = isBound();
    unbind();
    m_Port = newPort;
    if (was_bound) {
        bind();
    }
}
const sf::Socket::Status Networking::SocketUDP::bind(const string& ip) {
    const auto status = m_Socket.bind(m_Port, internal_ip(ip));
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_Socket.getLocalPort(), 0, internal_ip(ip), SocketType::UDP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
void Networking::SocketUDP::unbind() { 
    if (isBound()) {
        EventSocket e = EventSocket(m_Socket.getLocalPort(), 0, internal_ip(m_IP), SocketType::UDP);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);

        m_Socket.unbind();

        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    //TODO: is this needed outside the if block?
    //m_Socket.unbind();
}
const sf::Socket::Status Networking::SocketUDP::send(sf::Packet& packet, const string& ip) {
    return Networking::SocketUDP::send(m_Port, packet, ip);
}
const sf::Socket::Status Networking::SocketUDP::send(const void* data, size_t size, const string& ip) {
    return m_Socket.send(data, size, internal_ip(ip), m_Port);
}
const sf::Socket::Status Networking::SocketUDP::receive(sf::Packet& packet) {
    sf::IpAddress ip;
    unsigned short port;
    const auto status = m_Socket.receive(packet, ip, port);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
const sf::Socket::Status Networking::SocketUDP::receive(void* data, size_t size, size_t received) {
    sf::IpAddress ip; 
    unsigned short port;
    const auto status = m_Socket.receive(data, size, received, ip, port);
    return status;
}

const sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, sf::Packet& packet, const string& ip) {
    UDPPacketInfo data;
    data.packet = packet;
    data.ip     = ip;
    data.port   = port;

    m_PartialPackets.push(std::move(data));
    auto& front = m_PartialPackets.front();
    const auto status = internal_send_packet(front);
    if (status == sf::Socket::Status::Done) {
        m_PartialPackets.pop();
    }
    return status;
}
const sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, const void* data, size_t size, const string& ip) {
    return m_Socket.send(data, size, internal_ip(ip), port);
}

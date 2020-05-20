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

    Core::m_Engine->m_NetworkingModule.m_SocketManager.add_udp_socket(this);
}
Networking::SocketUDP::~SocketUDP() { 
    Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_udp_socket(this);
}

sf::Socket::Status Networking::SocketUDP::internal_send_packet(UDPPacketInfo& packet) {
    auto status = m_SocketUDP.send(packet.packet, packet.ip, packet.port);
    switch (status) {
        case sf::Socket::Status::Done: {
            EventPacket e(&packet.packet);
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
sf::Socket::Status Networking::SocketUDP::internal_send_partial_packets_loop() {
    //every frame, send a partial packet. TODO: do this in a while loop?
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
void Networking::SocketUDP::update(const float dt) {
    if (isBound()) {
        auto status = internal_send_partial_packets_loop();
    }else{
        //TODO: add a notification that this socket is no longer bound?
    }
}
void Networking::SocketUDP::changePort(const unsigned short newPort) {
    bool was_bound = isBound();
    unbind();
    m_Port = newPort;
    if (was_bound) {
        bind();
    }
}
sf::Socket::Status Networking::SocketUDP::bind(const string& ip) {
    auto status = m_SocketUDP.bind(m_Port, internal_get_ip(ip));
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_SocketUDP.getLocalPort(), 0, internal_get_ip(ip), SocketType::UDP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
void Networking::SocketUDP::unbind() { 
    if (isBound()) {
        EventSocket e = EventSocket(m_SocketUDP.getLocalPort(), 0, internal_get_ip(m_IP), SocketType::UDP);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);

        m_SocketUDP.unbind();

        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
}
sf::Socket::Status Networking::SocketUDP::send(Engine::Networking::Packet& packet, const string& ip) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    return send(sf_packet, ip);
}
sf::Socket::Status Networking::SocketUDP::send(sf::Packet& packet, const string& ip) {
    return Networking::SocketUDP::send(m_Port, packet, ip);
}
sf::Socket::Status Networking::SocketUDP::send(const void* data, size_t size, const string& ip) {
    return m_SocketUDP.send(data, size, internal_get_ip(ip), m_Port);
}
sf::Socket::Status Networking::SocketUDP::receive(sf::Packet& packet) {
    sf::IpAddress ip;
    unsigned short port;
    auto status = m_SocketUDP.receive(packet, ip, port);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    }
    return status;
}
sf::Socket::Status Networking::SocketUDP::receive(void* data, size_t size, size_t& received) {
    sf::IpAddress ip; 
    unsigned short port;
    auto status = m_SocketUDP.receive(data, size, received, ip, port);
    return status;
}
sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, Engine::Networking::Packet& packet, const string& ip) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    return send(port, sf_packet, ip);
}
sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, sf::Packet& packet, const string& ip) {
    UDPPacketInfo data;
    data.packet = packet;
    data.ip     = ip;
    data.port   = port;

    m_PartialPackets.push(std::move(data));
    auto status = internal_send_partial_packets_loop();
    return status;
}
sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, const void* data, size_t size, const string& ip) {
    return m_SocketUDP.send(data, size, internal_get_ip(ip), port);
}

#pragma region Getters / Setters

sf::IpAddress Networking::SocketUDP::internal_get_ip(const string& ip) const {
    return (ip.empty()) ? sf::IpAddress::Any : ip;
}
bool Networking::SocketUDP::isBound() const {
    return (m_SocketUDP.getLocalPort() != 0);
}
//sf::UdpSocket& Networking::SocketUDP::getSFMLSocket() {
//    return m_SocketUDP;
//}
unsigned short Networking::SocketUDP::localPort() const {
    return m_SocketUDP.getLocalPort();
}
void Networking::SocketUDP::setBlocking(bool blocking) {
    m_SocketUDP.setBlocking(blocking);
}
bool Networking::SocketUDP::isBlocking() const {
    return m_SocketUDP.isBlocking();
}

#pragma endregion
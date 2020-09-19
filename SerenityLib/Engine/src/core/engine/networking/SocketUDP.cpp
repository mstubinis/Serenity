#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/SocketUDP.h>
#include <core/engine/events/EventModule.h>
#include <core/engine/events/EventIncludes.h>
#include <core/engine/events/Event.h>
#include <core/engine/system/Engine.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;

Networking::SocketUDP::UDPPacketInfo::UDPPacketInfo(sf::Packet* inSFMLPacket, unsigned short inPort, sf::IpAddress&& inAddress) 
    : port{ inPort }
    , ip{ std::move(inAddress) }
    , sfmlPacket{ std::unique_ptr<sf::Packet>(inSFMLPacket) }
{}
Networking::SocketUDP::UDPPacketInfo::UDPPacketInfo(Networking::SocketUDP::UDPPacketInfo&& other) noexcept 
    : port{ std::move(other.port) }
    , ip{ std::move(other.ip) }
{
    sfmlPacket.swap(other.sfmlPacket);
}
Networking::SocketUDP::UDPPacketInfo& Networking::SocketUDP::UDPPacketInfo::operator=(Networking::SocketUDP::UDPPacketInfo&& other) noexcept {
    if (&other != this) {
        port       = std::move(other.port);
        ip         = std::move(other.ip);
        sfmlPacket.swap(other.sfmlPacket);
    }
    return *this;
}
Networking::SocketUDP::UDPPacketInfo::~UDPPacketInfo() {
}




Networking::SocketUDP::SocketUDP(unsigned short port, const std::string& ip) 
    : m_Port(port)
    , m_IP(ip)
{
    Core::m_Engine->m_NetworkingModule.m_SocketManager.add_udp_socket(this);
}
Networking::SocketUDP::~SocketUDP() { 
    Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_udp_socket(this);
}
void Networking::SocketUDP::clearPartialPackets() {
    while (!m_PartialPackets.empty()) {
        m_PartialPackets.pop();
    }
}
SocketStatus::Status Networking::SocketUDP::internal_send_packet(UDPPacketInfo& PacketInfoStruct) {
    auto status = m_SocketUDP.send(*PacketInfoStruct.sfmlPacket, PacketInfoStruct.ip, PacketInfoStruct.port);
    switch (status) {
        case sf::Socket::Status::Done: {
            EventPacket e(PacketInfoStruct.sfmlPacket.get());
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
SocketStatus::Status Networking::SocketUDP::internal_send_partial_packets_loop() {
    //every frame, send a partial packet. TODO: do this in a while loop?
    SocketStatus::Status status = SocketStatus::Error;
    if (!m_PartialPackets.empty()) {
        auto& front = m_PartialPackets.front();
        status = internal_send_packet(front);
        switch (status) {
            case SocketStatus::Done: {
                m_PartialPackets.pop();
                break;
            }case SocketStatus::Disconnected: {
                m_PartialPackets.pop();
                break;
            }case SocketStatus::Error: {
                m_PartialPackets.pop();
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
void Networking::SocketUDP::update(const float dt) {
    if (isBound()) {
        auto status = internal_send_partial_packets_loop();
    }else{
        //TODO: add a notification that this socket is no longer bound?
    }
}
void Networking::SocketUDP::changePort(unsigned short newPort) {
    bool was_bound = isBound();
    unbind();
    m_Port = newPort;
    if (was_bound) {
        bind();
    }
}
SocketStatus::Status Networking::SocketUDP::bind(const std::string& ip) {
    auto status = m_SocketUDP.bind(m_Port, internal_get_ip(ip));
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_SocketUDP.getLocalPort(), 0, internal_get_ip(ip), SocketType::UDP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
void Networking::SocketUDP::unbind() { 
    if (isBound()) {
        clearPartialPackets();

        EventSocket e = EventSocket(m_SocketUDP.getLocalPort(), 0, internal_get_ip(m_IP), SocketType::UDP);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);

        m_SocketUDP.unbind();

        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Networking::SocketUDP::send(Engine::Networking::Packet& packet, const std::string& ip) {
    return Networking::SocketUDP::send(m_Port, packet, ip);
}
SocketStatus::Status Networking::SocketUDP::send(sf::Packet& sfpacket, const std::string& ip) {
    return Networking::SocketUDP::send(m_Port, sfpacket, ip);
}
SocketStatus::Status Networking::SocketUDP::send(const void* data, size_t size, const std::string& ip) {
    return SocketStatus::map_status(m_SocketUDP.send(data, size, internal_get_ip(ip), m_Port));
}
SocketStatus::Status Networking::SocketUDP::receive(Engine::Networking::Packet& packet, sf::IpAddress& ip, unsigned short& port) {
    auto status = m_SocketUDP.receive(packet, ip, port);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketUDP::receive(sf::Packet& sfpacket, sf::IpAddress& ip, unsigned short& port) {
    auto status = m_SocketUDP.receive(sfpacket, ip, port);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&sfpacket);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketUDP::receive(void* data, size_t size, size_t& received, sf::IpAddress& ip, unsigned short& port) {
    return SocketStatus::map_status(m_SocketUDP.receive(data, size, received, ip, port));
}
SocketStatus::Status Networking::SocketUDP::send(unsigned short port, Engine::Networking::Packet& packet, const std::string& ip) {
    sf::IpAddress dataIP = ip;
    m_PartialPackets.emplace(packet.clone(), port, std::move(dataIP));
    auto status = internal_send_partial_packets_loop();
    return status;
}
SocketStatus::Status Networking::SocketUDP::send(unsigned short port, sf::Packet& sfpacket, const std::string& ip) {
    sf::IpAddress dataIP = ip;
    m_PartialPackets.emplace(NEW sf::Packet(sfpacket), port, std::move(dataIP));
    auto status = internal_send_partial_packets_loop();
    return status;
}
SocketStatus::Status Networking::SocketUDP::send(unsigned short port, const void* data, size_t size, const std::string& ip) {
    return SocketStatus::map_status(m_SocketUDP.send(data, size, internal_get_ip(ip), port));
}

#pragma region Getters / Setters

sf::IpAddress Networking::SocketUDP::internal_get_ip(const std::string& ip) const {
    return (ip.empty()) ? sf::IpAddress::Any : ip;
}
bool Networking::SocketUDP::isBound() const {
    return (m_SocketUDP.getLocalPort() != 0);
}
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
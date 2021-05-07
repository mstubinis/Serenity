
#include <serenity/networking/SocketUDP.h>
#include <serenity/events/EventModule.h>
#include <serenity/events/EventIncludes.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>
#include <serenity/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;

Networking::SocketUDP::SocketUDP(uint16_t port, sf::IpAddress ip)
    : m_Port{ port }
    , m_IP{ ip }
{
    Core::m_Engine->m_NetworkingModule.m_SocketManager.add_udp_socket(this);
}
Networking::SocketUDP::~SocketUDP() { 
    unbind();
    Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_udp_socket(this);
}
void Networking::SocketUDP::clearPartialPackets() {
    while (!m_PartialPackets.empty()) {
        m_PartialPackets.pop_front();
    }
}
uint32_t Networking::SocketUDP::getNumPartialPackets() const noexcept {
    return (uint32_t)m_PartialPackets.size();
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
                m_PartialPackets.pop_front();
                break;
            }case SocketStatus::Disconnected: {
                m_PartialPackets.pop_front();
                break;
            }case SocketStatus::Error: {
                m_PartialPackets.pop_front();
                break;
            }case SocketStatus::NotReady: {
                break;
            }case SocketStatus::Partial: {
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
void Networking::SocketUDP::changePort(uint16_t newPort) {
    bool was_bound = isBound();
    unbind();
    m_Port = newPort;
    if (was_bound) {
        bind();
    }
}
SocketStatus::Status Networking::SocketUDP::bind(sf::IpAddress ip) {
    auto status = m_SocketUDP.bind(m_Port, ip);
    if (status == sf::Socket::Status::Done) {
        EventSocket e = EventSocket(m_SocketUDP.getLocalPort(), 0, ip, SocketType::UDP);
        Event ev(EventType::SocketConnected);
        ev.eventSocket = std::move(e);
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
void Networking::SocketUDP::unbind() { 
    if (isBound()) {
        clearPartialPackets();

        EventSocket e = EventSocket(m_SocketUDP.getLocalPort(), 0, m_IP, SocketType::UDP);
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = std::move(e);

        m_SocketUDP.unbind();

        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Networking::SocketUDP::receive(Engine::Networking::Packet& packet, sf::IpAddress& ip, uint16_t& port) {
    auto status = m_SocketUDP.receive(packet, ip, port);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&packet);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketUDP::receive(sf::Packet& sfpacket, sf::IpAddress& ip, uint16_t& port) {
    auto status = m_SocketUDP.receive(sfpacket, ip, port);
    if (status == sf::Socket::Status::Done) {
        EventPacket e(&sfpacket);
        Event ev(EventType::PacketReceived);
        ev.eventPacket = e;
        Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Networking::SocketUDP::receive(void* data, size_t size, size_t& received, sf::IpAddress& ip, uint16_t& port) {
    return SocketStatus::map_status(m_SocketUDP.receive(data, size, received, ip, port));
}



SocketStatus::Status Networking::SocketUDP::send(Engine::Networking::Packet& packet, sf::IpAddress ip) {
    return Networking::SocketUDP::send(m_Port, packet, ip);
}
SocketStatus::Status Networking::SocketUDP::send(sf::Packet& sfpacket, sf::IpAddress ip) {
    return Networking::SocketUDP::send(m_Port, sfpacket, ip);
}
SocketStatus::Status Networking::SocketUDP::send(const void* data, size_t size, sf::IpAddress ip) {
    return SocketStatus::map_status(m_SocketUDP.send(data, size, ip, m_Port));
}
SocketStatus::Status Networking::SocketUDP::send(uint16_t port, Engine::Networking::Packet& packet, sf::IpAddress ip) {
    m_PartialPackets.emplace_back(port, std::move(ip), packet.clone());
    auto status = internal_send_partial_packets_loop();
    return status;
}
SocketStatus::Status Networking::SocketUDP::send(uint16_t port, sf::Packet& sfpacket, sf::IpAddress ip) {
    m_PartialPackets.emplace_back(port, std::move(ip), NEW sf::Packet(sfpacket));
    auto status = internal_send_partial_packets_loop();
    return status;
}
SocketStatus::Status Networking::SocketUDP::send(uint16_t port, const void* data, size_t size, sf::IpAddress ip) {
    return SocketStatus::map_status(m_SocketUDP.send(data, size, ip, port));
}









#pragma region Getters / Setters

bool Networking::SocketUDP::isBound() const {
    return (m_SocketUDP.getLocalPort() != 0);
}
uint16_t Networking::SocketUDP::localPort() const {
    return m_SocketUDP.getLocalPort();
}
void Networking::SocketUDP::setBlocking(bool blocking) {
    m_SocketUDP.setBlocking(blocking);
}
bool Networking::SocketUDP::isBlocking() const {
    return m_SocketUDP.isBlocking();
}

#pragma endregion

#include <serenity/networking/SocketUDP.h>
#include <serenity/events/EventDispatcher.h>
#include <serenity/events/EventIncludes.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>
#include <serenity/utils/Utils.h>

Engine::Networking::SocketUDP::SocketUDP(uint16_t port, sf::IpAddress ip)
    : m_IP{ ip }
    , m_Port{ port }
{
    Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.add_udp_socket(this);
}
Engine::Networking::SocketUDP::~SocketUDP() {
    unbind();
    Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_udp_socket(this);
}
void Engine::Networking::SocketUDP::clearPartialPackets() {
    while (!m_PartialPackets.empty()) {
        m_PartialPackets.pop_front();
    }
}
uint32_t Engine::Networking::SocketUDP::getNumPartialPackets() const noexcept {
    return (uint32_t)m_PartialPackets.size();
}
SocketStatus::Status Engine::Networking::SocketUDP::internal_send_packet(UDPPacketInfo& PacketInfoStruct) {
    auto status = m_SocketUDP.send(PacketInfoStruct.packet, PacketInfoStruct.ip, PacketInfoStruct.port);
    switch (status) {
        case sf::Socket::Status::Done: {
            break;
        } case sf::Socket::Status::Disconnected: {
            break;
        } case sf::Socket::Status::Error: {
            break;
        } case sf::Socket::Status::NotReady: {
            break;
        } case sf::Socket::Status::Partial: {
            break;
        }
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Engine::Networking::SocketUDP::internal_send_partial_packets_loop() {
    //every frame, send a partial packet. TODO: do this in a while loop?
    SocketStatus::Status status = SocketStatus::Error;
    if (!m_PartialPackets.empty()) {
        auto& front = m_PartialPackets.front();
        status = internal_send_packet(front);
        switch (status) {
            case SocketStatus::Done: {
                m_PartialPackets.pop_front();
                break;
            } case SocketStatus::Disconnected: {
                m_PartialPackets.pop_front();
                break;
            } case SocketStatus::Error: {
                m_PartialPackets.pop_front();
                break;
            } case SocketStatus::NotReady: {
                break;
            } case SocketStatus::Partial: {
                break;
            }
        }
    }
    return status;
}
void Engine::Networking::SocketUDP::update(const float dt) {
    if (isBound()) {
        internal_send_partial_packets_loop();
    } else {
        //TODO: add a notification that this socket is no longer bound?
    }
}
void Engine::Networking::SocketUDP::changePort(uint16_t newPort) {
    bool was_bound = isBound();
    unbind();
    m_Port = newPort;
    if (was_bound) {
        bind();
    }
}
SocketStatus::Status Engine::Networking::SocketUDP::bind(sf::IpAddress ip) {
    auto status = m_SocketUDP.bind(m_Port, ip);
    if (status == sf::Socket::Status::Done) {
        Event ev(EventType::SocketConnected);
        ev.eventSocket = Engine::priv::EventSocket{ m_SocketUDP.getLocalPort(), 0, ip, SocketType::UDP };
        Engine::priv::Core::m_Engine->m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
void Engine::Networking::SocketUDP::unbind() {
    if (isBound()) {
        clearPartialPackets();
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = Engine::priv::EventSocket{ m_SocketUDP.getLocalPort(), 0, m_IP, SocketType::UDP };

        m_SocketUDP.unbind();

        Engine::priv::Core::m_Engine->m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Engine::Networking::SocketUDP::receive(Engine::Networking::Packet& packet, sf::IpAddress& ip, uint16_t& port) {
    auto status = m_SocketUDP.receive(packet, ip, port);
    return SocketStatus::map_status(status);
}
SocketStatus::Status Engine::Networking::SocketUDP::receive(void* data, size_t size, size_t& received, sf::IpAddress& ip, uint16_t& port) {
    return SocketStatus::map_status(m_SocketUDP.receive(data, size, received, ip, port));
}



SocketStatus::Status Engine::Networking::SocketUDP::send(Engine::Networking::Packet& packet, sf::IpAddress ip) {
    return send(m_Port, packet, ip);
}
SocketStatus::Status Engine::Networking::SocketUDP::send(const void* data, size_t size, sf::IpAddress ip) {
    return SocketStatus::map_status(m_SocketUDP.send(data, size, ip, m_Port));
}
SocketStatus::Status Engine::Networking::SocketUDP::send(uint16_t port, Engine::Networking::Packet& packet, sf::IpAddress ip) {
    m_PartialPackets.emplace_back(port, std::move(ip), packet);
    auto status = internal_send_partial_packets_loop();
    return status;
}
SocketStatus::Status Engine::Networking::SocketUDP::send(uint16_t port, const void* data, size_t size, sf::IpAddress ip) {
    return SocketStatus::map_status(m_SocketUDP.send(data, size, ip, port));
}


#pragma region Getters / Setters

bool Engine::Networking::SocketUDP::isBound() const {
    return (m_SocketUDP.getLocalPort() != 0);
}
uint16_t Engine::Networking::SocketUDP::localPort() const {
    return m_SocketUDP.getLocalPort();
}
void Engine::Networking::SocketUDP::setBlocking(bool blocking) {
    m_SocketUDP.setBlocking(blocking);
}
bool Engine::Networking::SocketUDP::isBlocking() const {
    return m_SocketUDP.isBlocking();
}

#pragma endregion
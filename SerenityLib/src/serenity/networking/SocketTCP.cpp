
#include <serenity/networking/SocketTCP.h>
#include <serenity/events/EventModule.h>
#include <serenity/events/EventIncludes.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>
#include <serenity/utils/Utils.h>

Engine::Networking::SocketTCP::SocketTCP() {
    Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.add_tcp_socket(this);
}
Engine::Networking::SocketTCP::SocketTCP(uint16_t port, sf::IpAddress ip)
    : Networking::SocketTCP::SocketTCP{}
{
    m_IP    = std::move(ip);
    m_Port  = port;
}
Engine::Networking::SocketTCP::~SocketTCP() {
    Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.remove_tcp_socket(this);
    disconnect();
}

SocketStatus::Status Engine::Networking::SocketTCP::internal_send_packet(Engine::Networking::Packet& packet) {
    auto status = m_SocketTCP.send(packet);
    switch (status) {
        case sf::Socket::Status::Done: {
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
SocketStatus::Status Engine::Networking::SocketTCP::internal_send_partial_packets_loop() {
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
            }
        }
    }
    return status;
}
void Engine::Networking::SocketTCP::update(const float dt) {
    if (isConnected()) {
        auto status = internal_send_partial_packets_loop();
    }
}
void Engine::Networking::SocketTCP::disconnect() {
    if (isConnected()) {
        Event ev(EventType::SocketDisconnected);
        ev.eventSocket = Engine::priv::EventSocket(m_SocketTCP.getLocalPort(), m_SocketTCP.getRemotePort(), m_SocketTCP.getRemoteAddress(), SocketType::TCP);

        m_SocketTCP.disconnect();

        Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
}
SocketStatus::Status Engine::Networking::SocketTCP::connect(uint16_t timeout) {
    auto status = m_SocketTCP.connect(m_IP, m_Port, sf::seconds(timeout));
    if (status == sf::Socket::Status::Done) {
        Event ev(EventType::SocketConnected);
        ev.eventSocket = Engine::priv::EventSocket(m_SocketTCP.getLocalPort(), m_SocketTCP.getRemotePort(), m_SocketTCP.getRemoteAddress(), SocketType::TCP);
        Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    }
    return SocketStatus::map_status(status);
}
SocketStatus::Status Engine::Networking::SocketTCP::send(Engine::Networking::Packet& packet) {
    m_PartialPackets.push(packet);
    return internal_send_partial_packets_loop();
}
SocketStatus::Status Engine::Networking::SocketTCP::send(const void* data, size_t size) {
    return SocketStatus::map_status(m_SocketTCP.send(data, size));
}
SocketStatus::Status Engine::Networking::SocketTCP::send(const void* data, size_t size, size_t& sent) {
    return SocketStatus::map_status(m_SocketTCP.send(data, size, sent));
}
SocketStatus::Status Engine::Networking::SocketTCP::receive(Engine::Networking::Packet& packet) {
    auto status = m_SocketTCP.receive(packet);
    return SocketStatus::map_status(status);
}
SocketStatus::Status Engine::Networking::SocketTCP::receive(void* data, size_t size, size_t& received) {
    return SocketStatus::map_status(m_SocketTCP.receive(data, size, received));
}
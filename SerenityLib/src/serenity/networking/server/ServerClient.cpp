
#include <serenity/networking/server/ServerClient.h>
#include <serenity/networking/server/Server.h>
#include <serenity/networking/SocketTCP.h>
#include <serenity/networking/NetworkingIncludes.h>

using namespace Engine;
using namespace Engine::Networking;

ServerClient::ServerClient(const std::string& hash, Server& server, SocketTCP* tcp_socket, sf::IpAddress clientIP, uint16_t in_client_Port)
    : m_Server{ server }
    , m_Hash{ hash }
    , m_ConnectionState{ ConnectionState::Active }
{
    if (tcp_socket) {
        m_IP        = tcp_socket->ip();
        m_Port      = tcp_socket->remotePort();
        m_TcpSocket = std::unique_ptr<Engine::Networking::SocketTCP>(std::move(tcp_socket));
        m_TcpSocket->setBlocking(false);
    }else{
        m_IP   = std::move(clientIP);
        m_Port = in_client_Port;
    }
    registerEvent(EventType::ClientConnected);
    registerEvent(EventType::ClientDisconnected);
    registerEvent(EventType::ServerStarted);
    registerEvent(EventType::ServerShutdowned);
}
ServerClient::~ServerClient() {
    unregisterEventImmediate(EventType::ClientConnected);
    unregisterEventImmediate(EventType::ClientDisconnected);
    unregisterEventImmediate(EventType::ServerStarted);
    unregisterEventImmediate(EventType::ServerShutdowned);
}
uint32_t ServerClient::generate_nonce() const noexcept {
    return Engine::Networking::NetworkingHelpers::generate_nonce();
}
bool ServerClient::connect(uint16_t timeout) noexcept {
    if (!m_TcpSocket || !disconnected()) {
        return false;
    }
    auto status = m_TcpSocket->connect(timeout);
    if (status != sf::Socket::Status::Done) {
        return false;
    }
    m_ConnectionState = ConnectionState::Active;
    return true;
}
void ServerClient::disconnect() noexcept {
    if (m_TcpSocket) {
        m_TcpSocket->disconnect();
    }
    m_ConnectionState        = ConnectionState::Disconnected;
    m_Recovery_Timeout_Timer = m_Recovery_Timeout_Timer_Limit;
}
bool ServerClient::disconnected() const noexcept {
    return (m_ConnectionState == ConnectionState::Disconnected || m_Timeout_Timer >= m_Timeout_Timer_Limit || (m_TcpSocket && m_TcpSocket->localPort() == 0));
}
SocketStatus::Status ServerClient::sendTcp(Engine::Networking::Packet& packet) noexcept {
    return m_TcpSocket->send(packet);
}
SocketStatus::Status ServerClient::receiveTcp(Engine::Networking::Packet& packet) noexcept {
    return m_TcpSocket->receive(packet);
}
SocketStatus::Status ServerClient::sendUdp(Engine::Networking::Packet& sfPacket) noexcept {
    return m_Server.sendUdp(this, sfPacket);
}

void ServerClient::internal_on_receive_udp(Engine::Networking::Packet& packet) noexcept {
    internal_on_received_data();
    ServerClient::m_On_Received_UDP_Function(packet);
}
void ServerClient::internal_update_receive_tcp_packet() noexcept {
    if (!m_TcpSocket) {
        return;
    }
    Engine::Networking::Packet packet;
    auto status = receiveTcp(packet);
    switch (status) {
        case SocketStatus::Done: {
            internal_on_received_data();
            m_On_Received_TCP_Function(packet);
            break;
        } case SocketStatus::NotReady: {
            break;
        } case SocketStatus::Partial: {
            break;
        } case SocketStatus::Disconnected: {
            break;
        } case SocketStatus::Error: {
            break;
        }
    }
}
void ServerClient::internal_update_connection_state(const float dt) noexcept {
    switch (m_ConnectionState) {
        case ConnectionState::Active: {
            m_Timeout_Timer += dt;
            if (disconnected()) {
                m_On_Timed_Out_Function();
                m_ConnectionState = ConnectionState::Disconnected;
            }
            break;
        } case ConnectionState::Disconnected: {
            m_Recovery_Timeout_Timer += dt;
            if (m_Recovery_Timeout_Timer >= m_Recovery_Timeout_Timer_Limit) {
                m_On_Recovery_Timed_Out_Function();
                m_ConnectionState = ConnectionState::Inactive;
            }
            break;
        } case ConnectionState::Inactive: {
            break;
        }
    }
}
void ServerClient::internal_on_received_data() noexcept {
    m_Timeout_Timer          = 0.0f;
    m_Recovery_Timeout_Timer = 0.0f;
    m_ConnectionState        = ConnectionState::Active;
}
void ServerClient::update(const float dt) noexcept {
    internal_update_connection_state(dt);
    internal_update_receive_tcp_packet();
    m_Update_Function(dt);
}

#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/server/Server.h>
#include <core/engine/networking/SocketTCP.h>
#include <random>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

ServerClient::ServerClient(string& hash, Server& server, SocketTCP* tcp_socket, string& in_client_IP, unsigned short inclient_Port) : m_Server(server){
    m_Hash          = hash;
    if (tcp_socket) {
        m_IP        = (!tcp_socket->ip().empty() ? tcp_socket->ip() : in_client_IP);
        m_Port      = tcp_socket->remotePort();
        m_TcpSocket = std::move(tcp_socket);
        m_TcpSocket->setBlocking(false);
    }else{
        m_IP   = in_client_IP;
        m_Port = inclient_Port;
    }
    m_ConnectionState = ConnectionState::Active;

    registerEvent(EventType::ClientConnected);
    registerEvent(EventType::ClientDisconnected);
    registerEvent(EventType::ServerStarted);
    registerEvent(EventType::ServerShutdowned);
    registerEvent(EventType::PacketSent);
    registerEvent(EventType::PacketReceived);
}
ServerClient::~ServerClient() {
    unregisterEvent(EventType::ClientConnected);
    unregisterEvent(EventType::ClientDisconnected);
    unregisterEvent(EventType::ServerStarted);
    unregisterEvent(EventType::ServerShutdowned);
    unregisterEvent(EventType::PacketSent);
    unregisterEvent(EventType::PacketReceived);

    SAFE_DELETE(m_TcpSocket);
}
sf::Uint32 ServerClient::generate_nonce() const {
    std::random_device device;
    std::mt19937 mt(device());
    std::uniform_int_distribution<sf::Uint32> distribution;
    return distribution(mt);
}
bool ServerClient::connect(unsigned short timeout) {
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
void ServerClient::disconnect() {
    if (m_TcpSocket) {
        m_TcpSocket->disconnect();
        m_ConnectionState = ConnectionState::Disconnected;
    }
}
bool ServerClient::disconnected() const {
    return (m_ConnectionState == ConnectionState::Disconnected || m_Timeout_Timer >= m_Timeout_Timer_Limit || (m_TcpSocket && m_TcpSocket->localPort() == 0));
}
SocketStatus::Status ServerClient::send_tcp(Packet& packet) {
    return m_TcpSocket->send(packet);
}
SocketStatus::Status ServerClient::send_tcp(sf::Packet& packet) {
    return m_TcpSocket->send(packet);
}
SocketStatus::Status ServerClient::receive_tcp(sf::Packet& packet) {
    return m_TcpSocket->receive(packet);
}
SocketStatus::Status ServerClient::send_udp(Engine::Networking::Packet& packet) {
    return m_Server.send_udp_to_client(*this, packet);
}
SocketStatus::Status ServerClient::send_udp(sf::Packet& sfPacket) {
    return m_Server.send_udp_to_client(*this, sfPacket);
}
void ServerClient::receive_udp(SocketStatus::Status status, sf::Packet& packet, const float dt) {
    if (status == SocketStatus::Done) {
        internal_on_received_data();
        on_receive_udp(packet, dt);
    }
}
void ServerClient::internal_update_tcp(const float dt) {
    if (!m_TcpSocket) {
        return;
    }
    sf::Packet sf_packet;
    auto status = receive_tcp(sf_packet);
    switch (status) {
        case SocketStatus::Done: {
            internal_on_received_data();
            m_On_Received_TCP_Function(sf_packet, dt);
            break;
        }case SocketStatus::NotReady: {
            break;
        }case SocketStatus::Partial: {
            break;
        }case SocketStatus::Disconnected: {
            break;
        }case SocketStatus::Error: {
            break;
        }default: {
            break;
        }
    }
}
void ServerClient::internal_update_connection_state(const float dt) {
    switch (m_ConnectionState) {
        case ConnectionState::Active: {
            m_Timeout_Timer += dt;
            if (disconnected()) {
                m_On_Timed_Out_Function();
                m_ConnectionState = ConnectionState::Disconnected;
            }
            break;
        }case ConnectionState::Disconnected: {
            m_Recovery_Timeout_Timer += dt;
            if (m_Recovery_Timeout_Timer >= m_Recovery_Timeout_Timer_Limit) {
                m_On_Recovery_Timed_Out_Function();
                m_ConnectionState = ConnectionState::Inactive;
            }
            break;
        }case ConnectionState::Inactive: {
            break;
        }default: {
            break;
        }
    }
}
void ServerClient::internal_on_received_data() {
    m_Timeout_Timer          = 0.0f;
    m_Recovery_Timeout_Timer = 0.0f;
    if (m_ConnectionState != ConnectionState::Active) {
        m_ConnectionState = ConnectionState::Active;
    }
}
void ServerClient::update(const float dt) {
    internal_update_connection_state(dt);
    internal_update_tcp(dt);
    m_Update_Function(dt);
}


/*
SocketStatus::Status ServerClient::send_tcp(void* data, size_t size) {
    return m_TcpSocket->send(data, size);
}
SocketStatus::Status ServerClient::send_tcp(void* data, size_t size, size_t& sent) {
    return m_TcpSocket->send(data, size, sent);
}
SocketStatus::Status ServerClient::receive_tcp(void* data, size_t size, size_t& received) {
    return m_TcpSocket->receive(data, size, received);
}
void ServerClient::receive_udp(SocketStatus::Status status, void* data, size_t size, size_t& received, const float dt) {
    if (status == SocketStatus::Done) {
        internal_on_received_data();
        on_receive_udp(data, size, received, dt);
    }
}
*/
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/SocketTCP.h>
#include <random>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

Engine::Networking::ServerClient::ServerClient(const string& hash, Engine::Networking::Server& server, Engine::Networking::SocketTCP* tcp_socket, string& inIP, unsigned short inPort) : m_Server(server){
    m_Hash      = hash;
    if (tcp_socket) {
        m_IP        = (!tcp_socket->ip().empty() ? tcp_socket->ip() : inIP);
        m_Port      = tcp_socket->remotePort();
        m_TcpSocket = std::move(tcp_socket);
        m_TcpSocket->setBlocking(false);
    }else{
        m_IP   = inIP;
        m_Port = inPort;
    }

    registerEvent(EventType::ClientConnected);
    registerEvent(EventType::ClientDisconnected);
    registerEvent(EventType::ServerStarted);
    registerEvent(EventType::ServerShutdowned);
    registerEvent(EventType::PacketSent);
    registerEvent(EventType::PacketReceived);
}
Engine::Networking::ServerClient::~ServerClient() {
    unregisterEvent(EventType::ClientConnected);
    unregisterEvent(EventType::ClientDisconnected);
    unregisterEvent(EventType::ServerStarted);
    unregisterEvent(EventType::ServerShutdowned);
    unregisterEvent(EventType::PacketSent);
    unregisterEvent(EventType::PacketReceived);

    SAFE_DELETE(m_TcpSocket);
}
sf::Uint32 Engine::Networking::ServerClient::generate_nonce() const {
    std::random_device device;
    std::mt19937 mt(device());
    std::uniform_int_distribution<sf::Uint32> distribution;
    sf::Uint32 res = distribution(mt);
    return res;
}
void Engine::Networking::ServerClient::setUpdateFunction(std::function<void(const float dt)> function) {
    m_Update_Function = function;
}
const std::string& Engine::Networking::ServerClient::hash() const {
    return m_Hash;
}
Engine::Networking::SocketTCP* Engine::Networking::ServerClient::socket() const {
    return m_TcpSocket;
}
unsigned short Engine::Networking::ServerClient::port() const {
    return m_Port;
}
const std::string& Engine::Networking::ServerClient::ip() const {
    return m_IP;
}
bool Engine::Networking::ServerClient::connect(unsigned short timeout) {
    if (!m_TcpSocket || !disconnected()) {
        return false;
    }
    auto status = m_TcpSocket->connect(timeout);
    if (status != sf::Socket::Status::Done) {
        return false;
    }
    return true;
}
Engine::Networking::ServerClient::ConnectionState::State Engine::Networking::ServerClient::connectionState() const {
    return m_ConnectionState;
}
void Engine::Networking::ServerClient::disconnect() {
    if (m_TcpSocket) {
        m_TcpSocket->disconnect();
        m_ConnectionState = ConnectionState::Disconnected;
    }
}
bool Engine::Networking::ServerClient::disconnected() const {
    return (m_ConnectionState == ConnectionState::Disconnected || m_Timeout_Timer >= m_Timeout_Timer_Limit || socket()->localPort() == 0);
}
void Engine::Networking::ServerClient::setTimeoutTimerLimit(float limit) {
    m_Timeout_Timer_Limit = limit;
}
unsigned int Engine::Networking::ServerClient::id() const {
    return m_ID;
}
SocketStatus::Status Engine::Networking::ServerClient::send_tcp(Engine::Networking::Packet& packet) {
    return m_TcpSocket->send(packet);
}
SocketStatus::Status Engine::Networking::ServerClient::send_tcp(sf::Packet& packet) {
    return m_TcpSocket->send(packet);
}
SocketStatus::Status Engine::Networking::ServerClient::send_tcp(void* data, size_t size) {
    return m_TcpSocket->send(data, size);
}
SocketStatus::Status Engine::Networking::ServerClient::send_tcp(void* data, size_t size, size_t& sent) {
    return m_TcpSocket->send(data, size, sent);
}
SocketStatus::Status Engine::Networking::ServerClient::receive_tcp(sf::Packet& packet) {
    return m_TcpSocket->receive(packet);
}
SocketStatus::Status Engine::Networking::ServerClient::receive_tcp(void* data, size_t size, size_t& received) {
    return m_TcpSocket->receive(data, size, received);
}
void Engine::Networking::ServerClient::receive_udp(SocketStatus::Status status, sf::Packet& packet, const float dt) {
    if (status == SocketStatus::Done) {
        internal_on_received_data();
        on_receive_udp(packet, dt);
    }
}
void Engine::Networking::ServerClient::receive_udp(SocketStatus::Status status, void* data, size_t size, size_t& received, const float dt) {
    if (status == SocketStatus::Done) {
        internal_on_received_data();
        on_receive_udp(data, size, received, dt);
    }
}

void Engine::Networking::ServerClient::internal_update_loop(const float dt) {
    switch (m_ConnectionState) {
        case ConnectionState::Active: {
            m_Timeout_Timer += dt;
            if (disconnected()) {
                on_timed_out();
                m_ConnectionState = ConnectionState::Disconnected;
            }
            break;
        }case ConnectionState::Disconnected: {
            m_Recovery_Timeout_Timer += dt;
            if (m_Recovery_Timeout_Timer >= m_Recovery_Timeout_Timer_Limit) {
                on_recovery_timed_out();
                m_ConnectionState = ConnectionState::Inactive;
            }
            break;
        }case ConnectionState::Inactive: {
            break;
        }default: {
            break;
        }
    }

    //tcp
    if (m_TcpSocket) {
        sf::Packet sf_packet;
        auto status = receive_tcp(sf_packet);
        if (status == SocketStatus::Done) {
            internal_on_received_data();
            on_receive_tcp(sf_packet, dt);
        }
    }
}
void Engine::Networking::ServerClient::internal_on_received_data() {
    m_Timeout_Timer          = 0.0f;
    m_Recovery_Timeout_Timer = 0.0f;
    if (m_ConnectionState != ConnectionState::Active) {
        m_ConnectionState = ConnectionState::Active;
    }
}
void Engine::Networking::ServerClient::update(const float dt) {
    internal_update_loop(dt);
    m_Update_Function(dt);
}

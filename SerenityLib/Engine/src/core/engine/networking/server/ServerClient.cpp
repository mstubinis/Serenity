#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/SocketTCP.h>
#include <random>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

ServerClient::ServerClient(const string& hash, Engine::Networking::Server& server, Engine::Networking::SocketTCP* tcp_socket) : m_Server(server){
    m_Hash      = hash;
    m_IP        = tcp_socket->ip();
    m_Port      = tcp_socket->remotePort();
    m_TcpSocket = std::move(tcp_socket);
    m_TcpSocket->setBlocking(false);

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
    sf::Uint32 res = distribution(mt);
    return res;
}
void ServerClient::setUpdateFunction(std::function<void(const float dt)> function) {
    m_Update_Function = function;
}
const std::string& ServerClient::hash() const {
    return m_Hash;
}
Engine::Networking::SocketTCP* ServerClient::socket() const {
    return m_TcpSocket;
}
unsigned short ServerClient::port() const {
    return m_Port;
}
const std::string& ServerClient::ip() const {
    return m_IP;
}
bool ServerClient::connect(unsigned short timeout) {
    if (!disconnected()) {
        return false;
    }
    auto status = m_TcpSocket->connect(timeout);
    if (status != sf::Socket::Status::Done) {
        return false;
    }
    return true;
}
ServerClient::ConnectionState::State ServerClient::connectionState() const {
    return m_ConnectionState;
}
void ServerClient::disconnect() {
    m_TcpSocket->disconnect();
    m_ConnectionState = ConnectionState::Disconnected;
}
bool ServerClient::disconnected() const {
    return (m_ConnectionState == ConnectionState::Disconnected || m_Timeout_Timer >= m_Timeout_Timer_Limit || socket()->localPort() == 0);
}
void ServerClient::setTimeoutTimerLimit(float limit) {
    m_Timeout_Timer_Limit = limit;
}
unsigned int ServerClient::id() const {
    return m_ID;
}
sf::Socket::Status ServerClient::send(Engine::Networking::Packet& packet) {
    return m_TcpSocket->send(packet);
}
sf::Socket::Status ServerClient::send(sf::Packet& packet) {
    return m_TcpSocket->send(packet);
}
sf::Socket::Status ServerClient::send(void* data, size_t size) {
    return m_TcpSocket->send(data, size);
}
sf::Socket::Status ServerClient::send(void* data, size_t size, size_t& sent) {
    return m_TcpSocket->send(data, size, sent);
}
sf::Socket::Status ServerClient::receive(sf::Packet& packet) {
    return m_TcpSocket->receive(packet);
}
sf::Socket::Status ServerClient::receive(void* data, size_t size, size_t& received) {
    return m_TcpSocket->receive(data, size, received);
}
void ServerClient::internal_update_loop(const float dt) {
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
    sf::Packet sf_packet;
    auto status = receive(sf_packet);
    if (status == sf::Socket::Done) {
        internal_on_received_data();
        on_receive_tcp(sf_packet, dt);
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
    internal_update_loop(dt);
    m_Update_Function(dt);
}
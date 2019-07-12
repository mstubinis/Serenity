#include <core/engine/networking/SocketTCP.h>
#include <core/engine/Engine_Utils.h>

using namespace Engine;
using namespace std;


Networking::SocketTCP::SocketTCP(const ushort port, const std::string& ip) {  //client side socket
    m_Socket    = new sf::TcpSocket();
    m_IP        = ip;
    m_Port      = port;
    m_Connected = false;
}
Networking::SocketTCP::SocketTCP(sf::TcpSocket* socket) { //server side client socket
    m_Socket    = socket;
    m_IP        = socket->getRemoteAddress().toString();
    m_Port      = socket->getRemotePort();
    m_Connected = true;
}

Networking::SocketTCP::~SocketTCP() { 
    disconnect(); 
}
const bool& Networking::SocketTCP::connected() const {
    return m_Connected;
}
const sf::TcpSocket& Networking::SocketTCP::socket() {
    return *m_Socket;
}
const string Networking::SocketTCP::ip() {
    return m_Socket->getRemoteAddress().toString();
}
const ushort Networking::SocketTCP::remotePort() {
    return m_Socket->getRemotePort();
}
const ushort Networking::SocketTCP::localPort() {
    return m_Socket->getLocalPort();
}
void Networking::SocketTCP::setBlocking(const bool b) {
    m_Socket->setBlocking(b);
}
const bool Networking::SocketTCP::isBlocking() {
    return m_Socket->isBlocking();
}

void Networking::SocketTCP::disconnect() { 
    SAFE_DELETE(m_Socket);
    m_Connected = false;
}
const sf::Socket::Status Networking::SocketTCP::connect(const ushort& timeout) {
    const auto& status = m_Socket->connect(m_IP, m_Port, sf::seconds(timeout));
    if (status != sf::Socket::Status::Done) {
        //error
    }else{
        m_Connected = true;
    }
    return status;
}
const sf::Socket::Status Networking::SocketTCP::send(sf::Packet& _packet) {
    if (!m_Connected) 
        return sf::Socket::Status::Disconnected;
    return m_Socket->send(_packet);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* _data, size_t _size) {
    if (!m_Connected)
        return sf::Socket::Status::Disconnected;
    return m_Socket->send(_data, _size);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* _data, size_t _size, size_t& _sent) {
    if (!m_Connected)
        return sf::Socket::Status::Disconnected;
    return m_Socket->send(_data, _size, _sent);
}
const sf::Socket::Status Networking::SocketTCP::receive(sf::Packet& _packet) {
    if (!m_Connected)
        return sf::Socket::Status::Disconnected;
    return m_Socket->receive(_packet);
}
const sf::Socket::Status Networking::SocketTCP::receive(void* _data, size_t _size, size_t& _sent) {
    if (!m_Connected)
        return sf::Socket::Status::Disconnected;
    return m_Socket->receive(_data, _size, _sent);
}
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;


Networking::SocketTCP::SocketTCP(const unsigned short port, const std::string& ip) {  //client side socket
    m_IP        = ip;
    m_Port      = port;
}

Networking::SocketTCP::~SocketTCP() { 
    m_Socket.disconnect();
}
const sf::TcpSocket& Networking::SocketTCP::socket() {
    return m_Socket;
}
const string Networking::SocketTCP::ip() {
    return m_Socket.getRemoteAddress().toString();
}
const unsigned short Networking::SocketTCP::remotePort() {
    return m_Socket.getRemotePort();
}
const unsigned short Networking::SocketTCP::localPort() {
    return m_Socket.getLocalPort();
}
void Networking::SocketTCP::setBlocking(const bool b) {
    m_Socket.setBlocking(b);
}
const bool Networking::SocketTCP::isBlocking() {
    return m_Socket.isBlocking();
}
void Networking::SocketTCP::disconnect() {
    m_Socket.disconnect();
}
const sf::Socket::Status Networking::SocketTCP::connect(const unsigned short timeout) {
    return m_Socket.connect(m_IP, m_Port, sf::seconds(timeout));
}
const sf::Socket::Status Networking::SocketTCP::send(sf::Packet& packet) {
    return m_Socket.send(packet);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* data, size_t size) {
    return m_Socket.send(data, size);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* data, size_t size, size_t sent) {
    return m_Socket.send(data, size, sent);
}
const sf::Socket::Status Networking::SocketTCP::receive(sf::Packet& packet) {
    return m_Socket.receive(packet);
}
const sf::Socket::Status Networking::SocketTCP::receive(void* data, size_t size, size_t sent) {
    return m_Socket.receive(data, size, sent);
}
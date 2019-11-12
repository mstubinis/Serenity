#include <core/engine/networking/SocketTCP.h>

using namespace Engine;
using namespace std;


Networking::SocketTCP::SocketTCP(const unsigned short port, const std::string& ip) {  //client side socket
    m_Socket    = new sf::TcpSocket();
    m_IP        = ip;
    m_Port      = port;
}
Networking::SocketTCP::SocketTCP(sf::TcpSocket* socket) { //server side client socket
    m_Socket    = socket;
    m_IP        = socket->getRemoteAddress().toString();
    m_Port      = socket->getRemotePort();
}

Networking::SocketTCP::~SocketTCP() { 
    delete m_Socket;
}
const sf::TcpSocket& Networking::SocketTCP::socket() {
    return *m_Socket;
}
const string Networking::SocketTCP::ip() {
    return m_Socket->getRemoteAddress().toString();
}
const unsigned short Networking::SocketTCP::remotePort() {
    return m_Socket->getRemotePort();
}
const unsigned short Networking::SocketTCP::localPort() {
    return m_Socket->getLocalPort();
}
void Networking::SocketTCP::setBlocking(const bool b) {
    m_Socket->setBlocking(b);
}
const bool Networking::SocketTCP::isBlocking() {
    return m_Socket->isBlocking();
}
void Networking::SocketTCP::disconnect() {
    m_Socket->disconnect();
}
const sf::Socket::Status Networking::SocketTCP::connect(const unsigned short& timeout) {
    return m_Socket->connect(m_IP, m_Port, sf::seconds(timeout));
}
const sf::Socket::Status Networking::SocketTCP::send(sf::Packet& _packet) {
    return m_Socket->send(_packet);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* _data, size_t _size) {
    return m_Socket->send(_data, _size);
}
const sf::Socket::Status Networking::SocketTCP::send(const void* _data, size_t _size, size_t& _sent) {
    return m_Socket->send(_data, _size, _sent);
}
const sf::Socket::Status Networking::SocketTCP::receive(sf::Packet& _packet) {
    return m_Socket->receive(_packet);
}
const sf::Socket::Status Networking::SocketTCP::receive(void* _data, size_t _size, size_t& _sent) {
    return m_Socket->receive(_data, _size, _sent);
}
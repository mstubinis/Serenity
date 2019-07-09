#include <core/engine/networking/SocketTCP.h>



using namespace Engine;
using namespace std;


Networking::SocketTCP::SocketTCP(const uint _port, const string& _ip, uint _timeout) {
    m_IP      = _ip;
    m_Port    = _port;
    m_Timeout = _timeout;
}
Networking::SocketTCP::~SocketTCP() { 
    disconnect(); 
}

const sf::TcpSocket& Networking::SocketTCP::socket() {
    return m_Socket;
}
const string Networking::SocketTCP::ip() {
    return m_Socket.getRemoteAddress().toString();
}
const ushort Networking::SocketTCP::remotePort() {
    return m_Socket.getRemotePort();
}
const ushort Networking::SocketTCP::localPort() {
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
void Networking::SocketTCP::connect() {
    m_Socket.disconnect();
    const sf::Socket::Status status = m_Socket.connect(m_IP, m_Port, sf::seconds(m_Timeout));
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketTCP::send(sf::Packet& _packet) {
    sf::Socket::Status status = m_Socket.send(_packet);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketTCP::send(const void* _data, size_t _size) {
    sf::Socket::Status status = m_Socket.send(_data, _size);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketTCP::send(const void* _data, size_t _size, size_t& _sent) {
    sf::Socket::Status status = m_Socket.send(_data, _size, _sent);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketTCP::receive(sf::Packet& _packet) {
    sf::Socket::Status status = m_Socket.receive(_packet);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketTCP::receive(void* _data, size_t _size, size_t& _sent) {
    sf::Socket::Status status = m_Socket.receive(_data, _size, _sent);
    if (status != sf::Socket::Done) {
        //error
    }
}
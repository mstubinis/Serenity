#include <core/engine/networking/SocketUDP.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

Networking::SocketUDP::SocketUDP(const unsigned short _port, const string& _ip){
    m_Socket    = NEW sf::UdpSocket();
    m_Port      = _port;
    m_IP        = sf::IpAddress(_ip);
}
Networking::SocketUDP::~SocketUDP() { 
    SAFE_DELETE(m_Socket);
}
const sf::UdpSocket& Networking::SocketUDP::socket() { 
    return *m_Socket; 
}
const unsigned short Networking::SocketUDP::localPort() {
    return m_Socket->getLocalPort(); 
}
void Networking::SocketUDP::setBlocking(bool b) { 
    m_Socket->setBlocking(b); 
}
const bool Networking::SocketUDP::isBlocking() { 
    return m_Socket->isBlocking(); 
}
const sf::Socket::Status Networking::SocketUDP::bind(const string& _ip) {
    unbind();
    sf::Socket::Status status;
    if (_ip.empty()) { 
        status = m_Socket->bind(m_Port, sf::IpAddress::Any);
    }else{ 
        status = m_Socket->bind(m_Port, _ip);
    }
    return status;
}
void Networking::SocketUDP::unbind() { 
    m_Socket->unbind();
}
const sf::Socket::Status Networking::SocketUDP::send(sf::Packet& _packet, const string& _ip) {
    if(_ip.empty())
        return m_Socket->send(_packet, m_IP, m_Port);
    else
        return m_Socket->send(_packet, _ip, m_Port);
}
const sf::Socket::Status Networking::SocketUDP::send(const void* _data, size_t _size, const string& _ip) {
    if (_ip.empty())
        return m_Socket->send(_data, _size, m_IP, m_Port);
    else
        return m_Socket->send(_data, _size, _ip, m_Port);
}
const sf::Socket::Status Networking::SocketUDP::receive(sf::Packet& _packet) {
    sf::IpAddress ip;
    unsigned short port;
    const auto status = m_Socket->receive(_packet, ip, port);
    return status;
}
const sf::Socket::Status Networking::SocketUDP::receive(void* _data, size_t _size, size_t& _received) {
    sf::IpAddress ip; 
    unsigned short port;
    const auto status = m_Socket->receive(_data, _size, _received, ip, port);
    return status;
}

const sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, sf::Packet& packet, const string& _ip) {
    if (_ip.empty())
        return m_Socket->send(packet, m_IP, port);
    else
        return m_Socket->send(packet, _ip, port);
}
const sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, const void* data, size_t size, const string& _ip) {
    if (_ip.empty())
        return m_Socket->send(data, size, m_IP, port);
    else
        return m_Socket->send(data, size, _ip, port);
}

#include <core/engine/networking/SocketUDP.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

Networking::SocketUDP::SocketUDP(const unsigned short port, const string& ip){
    m_Port      = port;
    m_IP        = sf::IpAddress(ip);
}
Networking::SocketUDP::~SocketUDP() { 
}
const sf::UdpSocket& Networking::SocketUDP::socket() { 
    return m_Socket; 
}
const unsigned short Networking::SocketUDP::localPort() {
    return m_Socket.getLocalPort(); 
}
void Networking::SocketUDP::setBlocking(bool b) { 
    m_Socket.setBlocking(b); 
}
const bool Networking::SocketUDP::isBlocking() { 
    return m_Socket.isBlocking(); 
}
const sf::Socket::Status Networking::SocketUDP::bind(const string& ip) {
    unbind();
    sf::Socket::Status status;
    if (ip.empty()) { 
        status = m_Socket.bind(m_Port, sf::IpAddress::Any);
    }else{ 
        status = m_Socket.bind(m_Port, ip);
    }
    return status;
}
void Networking::SocketUDP::unbind() { 
    m_Socket.unbind();
}
const sf::Socket::Status Networking::SocketUDP::send(sf::Packet& packet, const string& ip) {
    if(ip.empty())
        return m_Socket.send(packet, m_IP, m_Port);
    return m_Socket.send(packet, ip, m_Port);
}
const sf::Socket::Status Networking::SocketUDP::send(const void* data, size_t size, const string& ip) {
    if (ip.empty())
        return m_Socket.send(data, size, m_IP, m_Port);
    return m_Socket.send(data, size, ip, m_Port);
}
const sf::Socket::Status Networking::SocketUDP::receive(sf::Packet& packet) {
    sf::IpAddress ip;
    unsigned short port;
    const auto status = m_Socket.receive(packet, ip, port);
    return status;
}
const sf::Socket::Status Networking::SocketUDP::receive(void* data, size_t size, size_t received) {
    sf::IpAddress ip; 
    unsigned short port;
    const auto status = m_Socket.receive(data, size, received, ip, port);
    return status;
}

const sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, sf::Packet& packet, const string& ip) {
    if (ip.empty())
        return m_Socket.send(packet, m_IP, port);
    return m_Socket.send(packet, ip, port);
}
const sf::Socket::Status Networking::SocketUDP::send(const unsigned short port, const void* data, size_t size, const string& ip) {
    if (ip.empty())
        return m_Socket.send(data, size, m_IP, port);
    return m_Socket.send(data, size, ip, port);
}

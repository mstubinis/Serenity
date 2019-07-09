#include <core/engine/networking/SocketUDP.h>

using namespace Engine;
using namespace std;


Networking::SocketUDP::SocketUDP(const uint _port){
    m_Port = _port;
}
Networking::SocketUDP::~SocketUDP() { 
    unbind(); 
}
const sf::UdpSocket& Networking::SocketUDP::socket() { 
    return m_Socket; 
}
const ushort Networking::SocketUDP::localPort() { 
    return m_Socket.getLocalPort(); 
}
void Networking::SocketUDP::setBlocking(bool b) { 
    m_Socket.setBlocking(b); 
}
const bool Networking::SocketUDP::isBlocking() { 
    return m_Socket.isBlocking(); 
}
void Networking::SocketUDP::bind(const string& _ip) {

    unbind();
    sf::Socket::Status status;
    if (_ip == "") { 
        status = m_Socket.bind(m_Port, sf::IpAddress(0, 0, 0, 0));
    }else{ 
        status = m_Socket.bind(m_Port, _ip);
    }
    //do whatever with status here
}
void Networking::SocketUDP::unbind() { 
    m_Socket.unbind();
}
void Networking::SocketUDP::send(sf::Packet& _packet, const string& _ip) {
    const sf::Socket::Status status = m_Socket.send(_packet, _ip, m_Port);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketUDP::send(const void* _data, size_t _size, const string& _ip) {
    const sf::Socket::Status status = m_Socket.send(_data, _size, _ip, m_Port);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketUDP::receive(sf::Packet& _packet, const string& _ip) {
    sf::IpAddress ip(_ip);
    const sf::Socket::Status status = m_Socket.receive(_packet, ip, m_Port);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::SocketUDP::receive(void* _data, size_t _size, size_t& _received, const string& _ip) {
    sf::IpAddress ip(_ip); 
    const sf::Socket::Status status = m_Socket.receive(_data, _size, _received, ip, m_Port);
    if (status != sf::Socket::Done) {
        //error
    }
}
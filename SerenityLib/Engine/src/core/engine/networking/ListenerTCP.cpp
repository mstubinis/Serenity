#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>

using namespace Engine;
using namespace std;

Networking::ListenerTCP::ListenerTCP(const unsigned short _port, const string& ip){
    m_Port = _port;
    m_Ip   = ip;
    m_Listener.setBlocking(false);
}
Networking::ListenerTCP::~ListenerTCP() { 
    close(); 
}
const sf::TcpListener& Networking::ListenerTCP::socket() { 
    return m_Listener; 
}
const unsigned short Networking::ListenerTCP::localPort() {
    return m_Listener.getLocalPort(); 
}
void Networking::ListenerTCP::setBlocking(bool b) { 
    m_Listener.setBlocking(b); 
}
const bool Networking::ListenerTCP::isBlocking() { 
    return m_Listener.isBlocking(); 
}
void Networking::ListenerTCP::close() { 
    m_Listener.close();
}
const sf::Socket::Status Networking::ListenerTCP::accept(sf::TcpSocket& client) {
    return m_Listener.accept(client);
}
const sf::Socket::Status Networking::ListenerTCP::accept(SocketTCP& client) {
    sf::TcpSocket& s = const_cast<sf::TcpSocket&>(client.socket());
    return m_Listener.accept(s);
}
const sf::Socket::Status Networking::ListenerTCP::listen() {
    sf::IpAddress _ip;
    if (m_Ip == "" || m_Ip == "0.0.0.0") {
        _ip = sf::IpAddress::Any;
    }else{
        _ip = sf::IpAddress(m_Ip);
    }
    return m_Listener.listen(m_Port, _ip);
}
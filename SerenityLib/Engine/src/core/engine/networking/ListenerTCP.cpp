#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>

#include <core/engine/Engine_Utils.h>

using namespace Engine;
using namespace std;


Networking::ListenerTCP::ListenerTCP(const uint _port, const string& _ip){
    m_IP = _ip;
    if (_ip == "")
        m_IP = "127.0.0.1";
    m_Port = _port;
    m_Listener.setBlocking(false);
}
Networking::ListenerTCP::~ListenerTCP() { 
    close(); 
}

const sf::TcpListener& Networking::ListenerTCP::socket() { 
    return m_Listener; 
}
const ushort Networking::ListenerTCP::localPort() { 
    return m_Listener.getLocalPort(); 
}
const std::string& Networking::ListenerTCP::ip() {
    return m_IP;
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

const sf::Socket::Status Networking::ListenerTCP::accept(sf::TcpSocket& _client) {
    return m_Listener.accept(_client);
}

const sf::Socket::Status Networking::ListenerTCP::accept(SocketTCP& _client) {
    sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client.socket());
    return m_Listener.accept(s);
}
const sf::Socket::Status Networking::ListenerTCP::listen() {
    return m_Listener.listen(m_Port, m_IP);
}
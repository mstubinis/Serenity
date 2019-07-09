#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/networking/SocketTCP.h>

using namespace Engine;
using namespace std;


Networking::ListenerTCP::ListenerTCP(const uint _port, const string& _ip){
    m_IP   = _ip;
    m_Port = _port;
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
void Networking::ListenerTCP::setBlocking(bool b) { 
    m_Listener.setBlocking(b); 
}
const bool Networking::ListenerTCP::isBlocking() { 
    return m_Listener.isBlocking(); 
}
void Networking::ListenerTCP::close() { 
    m_Listener.close(); 
}
void Networking::ListenerTCP::accept(SocketTCP& _client) {
    sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client.socket());
    sf::Socket::Status status = m_Listener.accept(s);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::ListenerTCP::accept(SocketTCP* _client) {
    sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client->socket());
    sf::Socket::Status status = m_Listener.accept(s);
    if (status != sf::Socket::Done) {
        //error
    }
}
void Networking::ListenerTCP::listen() {
    m_Listener.close();
    sf::Socket::Status status;
    if (m_IP == "") { 
        status = m_Listener.listen(m_Port, sf::IpAddress(0, 0, 0, 0));
    }else{ 
        status = m_Listener.listen(m_Port, m_IP);
    }
    if (status != sf::Socket::Done) {
        //error
    }
}
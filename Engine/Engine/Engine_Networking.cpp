#include "Engine_Networking.h"

using namespace Engine;
using namespace std;

class Networking::SocketTCP::impl{
    public:
        sf::TcpSocket m_Socket;
        string m_IP;
        ushort m_Port, m_Timeout;
        void _init(const string& _ip, const ushort _port,ushort _timeout){
            m_IP = _ip;
            m_Port = _port;
            m_Timeout = _timeout;
        }
        sf::Socket::Status _connect(){
            m_Socket.disconnect();
            return m_Socket.connect(m_IP, m_Port,sf::seconds(m_Timeout));
        }
        void _disconnect(){ m_Socket.disconnect(); }
        sf::Socket::Status _send(sf::Packet& _packet){ return m_Socket.send(_packet); }
        sf::Socket::Status _send(const void* _data, size_t _size){ return m_Socket.send(_data,_size); }
        sf::Socket::Status _send(const void* _data, size_t _size,size_t& _sent){ return m_Socket.send(_data,_size,_sent); }
        sf::Socket::Status _receive(sf::Packet& _packet){ return m_Socket.receive(_packet); }
        sf::Socket::Status _receive(void* _data, size_t _size,size_t& _received){ return m_Socket.receive(_data,_size,_received); }
};
Networking::SocketTCP::SocketTCP(const uint _port,const string& _ip,uint _timeout):m_i(new impl){
    m_i->_init(_ip,_port,_timeout);
}
Networking::SocketTCP::~SocketTCP(){ disconnect(); }
const sf::TcpSocket& Networking::SocketTCP::socket(){ return m_i->m_Socket; }
const string Networking::SocketTCP::ip(){ return m_i->m_Socket.getRemoteAddress().toString(); }
const ushort Networking::SocketTCP::remotePort(){ return m_i->m_Socket.getRemotePort(); }
const ushort Networking::SocketTCP::localPort(){ return m_i->m_Socket.getLocalPort(); }
void Networking::SocketTCP::connect(){
    sf::Socket::Status status = m_i->_connect();
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketTCP::disconnect(){ m_i->_disconnect(); }
void Networking::SocketTCP::setBlocking(bool b){ m_i->m_Socket.setBlocking(b); }
const bool Networking::SocketTCP::isBlocking(){ return m_i->m_Socket.isBlocking(); }
void Networking::SocketTCP::send(sf::Packet& _packet){
    sf::Socket::Status status = m_i->_send(_packet);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketTCP::send(const void* _data, size_t _size){
    sf::Socket::Status status = m_i->_send(_data,_size);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketTCP::send(const void* _data, size_t _size,size_t& _sent){
    sf::Socket::Status status = m_i->_send(_data,_size,_sent);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketTCP::receive(sf::Packet& _packet){
    sf::Socket::Status status = m_i->_receive(_packet);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketTCP::receive(void* _data, size_t _size,size_t& _sent){
    sf::Socket::Status status = m_i->_receive(_data,_size,_sent);
    if(status != sf::Socket::Done){
        //error
    }
}
class Networking::SocketUDP::impl{
    public:
        sf::UdpSocket m_Socket;
        ushort m_Port;
        void _init(ushort _port){
            m_Port = _port;
        }
        sf::Socket::Status _bind(const std::string& _ip){
            m_Socket.unbind();
            if(_ip == ""){ return m_Socket.bind(m_Port, sf::IpAddress(0, 0, 0, 0)); }
            else{ return m_Socket.bind(m_Port,_ip); }
        }
        void _unbind(){ m_Socket.unbind(); }
        sf::Socket::Status _send(sf::Packet& _packet,const string& _ip){ return m_Socket.send(_packet,_ip,m_Port); }
        sf::Socket::Status _send(const void* _data, size_t _size,const string& _ip){ return m_Socket.send(_data,_size,_ip,m_Port); }
        sf::Socket::Status _receive(sf::Packet& _packet,const string& _ip){ 
            sf::IpAddress ip(_ip); return m_Socket.receive(_packet,ip,m_Port);
        }
        sf::Socket::Status _receive(void* _data, size_t _size,size_t& _received,const string& _ip){ 
            sf::IpAddress ip(_ip); return m_Socket.receive(_data,_size,_received,ip,m_Port);
        }
};
Networking::SocketUDP::SocketUDP(const uint _port):m_i(new impl){
    m_i->_init(_port);
}
Networking::SocketUDP::~SocketUDP(){ unbind(); }
const sf::UdpSocket& Networking::SocketUDP::socket(){ return m_i->m_Socket; }
const ushort Networking::SocketUDP::localPort(){ return m_i->m_Socket.getLocalPort(); }
void Networking::SocketUDP::setBlocking(bool b){ m_i->m_Socket.setBlocking(b); }
const bool Networking::SocketUDP::isBlocking(){ return m_i->m_Socket.isBlocking(); }
void Networking::SocketUDP::bind(const string& _ip){
    sf::Socket::Status status = m_i->_bind(_ip);
}
void Networking::SocketUDP::unbind(){ m_i->_unbind(); }
void Networking::SocketUDP::send(sf::Packet& _packet,const string& _ip){
    sf::Socket::Status status = m_i->_send(_packet,_ip);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketUDP::send(const void* _data, size_t _size,const string& _ip){
    sf::Socket::Status status = m_i->_send(_data,_size,_ip);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketUDP::receive(sf::Packet& _packet,const string& _ip){
    sf::Socket::Status status = m_i->_receive(_packet,_ip);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::SocketUDP::receive(void* _data, size_t _size,size_t& _received,const string& _ip){
    sf::Socket::Status status = m_i->_receive(_data,_size,_received,_ip);
    if(status != sf::Socket::Done){
        //error
    }
}


class Networking::ListenerTCP::impl{
    public:
        sf::TcpListener m_Listener;
        string m_IP;
        ushort m_Port;
        void _init(const string& _ip, const uint _port){
            m_IP = _ip;
            m_Port = _port;
        }
        sf::Socket::Status _listen(){
            m_Listener.close();
            if(m_IP == ""){ return m_Listener.listen(m_Port,sf::IpAddress(0,0,0,0)); }
            else{ return m_Listener.listen(m_Port,m_IP); }
        }
        sf::Socket::Status _accept(sf::TcpSocket& _socket){
            return m_Listener.accept(_socket);
        }
};

Networking::ListenerTCP::ListenerTCP(const uint _port,const string& _ip):m_i(new impl){
    m_i->_init(_ip,_port);
}
Networking::ListenerTCP::~ListenerTCP(){ close(); }

const sf::TcpListener& Networking::ListenerTCP::socket(){ return m_i->m_Listener; }
const ushort Networking::ListenerTCP::localPort(){ return m_i->m_Listener.getLocalPort(); }
void Networking::ListenerTCP::setBlocking(bool b){ m_i->m_Listener.setBlocking(b); }
const bool Networking::ListenerTCP::isBlocking(){ return m_i->m_Listener.isBlocking(); }
void Networking::ListenerTCP::close(){ m_i->m_Listener.close(); }
void Networking::ListenerTCP::accept(SocketTCP& _client){
    sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client.socket());
    sf::Socket::Status status = m_i->_accept(s);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::ListenerTCP::accept(SocketTCP* _client){
    sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client->socket());
    sf::Socket::Status status = m_i->_accept(s);
    if(status != sf::Socket::Done){
        //error
    }
}
void Networking::ListenerTCP::listen(){
    sf::Socket::Status status = m_i->_listen();
    if(status != sf::Socket::Done){
        //error
    }
}

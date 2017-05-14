#include "Engine_Networking.h"

class SocketTCP::impl{
	public:
		sf::TcpSocket m_Socket;
		std::string m_IP;
		ushort m_Port;
		ushort m_Timeout;
		void _init(const std::string& _ip, const ushort _port,ushort _timeout){
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
SocketTCP::SocketTCP(const std::string& _ip, const uint _port,uint _timeout):m_i(new impl){
	m_i->_init(_ip,_port,_timeout);
}
SocketTCP::~SocketTCP(){ disconnect(); }
const sf::TcpSocket& SocketTCP::socket(){ return m_i->m_Socket; }
const std::string& SocketTCP::ip(){ return m_i->m_Socket.getRemoteAddress().toString(); }
const ushort SocketTCP::remotePort(){ return m_i->m_Socket.getRemotePort(); }
const ushort SocketTCP::localPort(){ return m_i->m_Socket.getLocalPort(); }
void SocketTCP::connect(){
	sf::Socket::Status status = m_i->_connect();
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketTCP::disconnect(){ m_i->_disconnect(); }
void SocketTCP::setBlocking(bool b){ m_i->m_Socket.setBlocking(b); }
const bool SocketTCP::isBlocking(){ return m_i->m_Socket.isBlocking(); }
void SocketTCP::send(sf::Packet& _packet){
	sf::Socket::Status status = m_i->_send(_packet);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketTCP::send(const void* _data, size_t _size){
	sf::Socket::Status status = m_i->_send(_data,_size);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketTCP::send(const void* _data, size_t _size,size_t& _sent){
	sf::Socket::Status status = m_i->_send(_data,_size,_sent);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketTCP::receive(sf::Packet& _packet){
	sf::Socket::Status status = m_i->_receive(_packet);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketTCP::receive(void* _data, size_t _size,size_t& _sent){
	sf::Socket::Status status = m_i->_receive(_data,_size,_sent);
	if(status != sf::Socket::Done){
		//error
	}
}
class SocketUDP::impl{
	public:
		sf::UdpSocket m_Socket;
		ushort m_Port;
		void _init(ushort _port){
			m_Port = _port;
		}
		sf::Socket::Status _bind(const std::string& _ip){
			m_Socket.unbind();
			if(_ip == ""){ return m_Socket.bind(m_Port,sf::IpAddress::Any); }
			else{ return m_Socket.bind(m_Port,_ip); }
		}
		void _unbind(){ m_Socket.unbind(); }
		sf::Socket::Status _send(sf::Packet& _packet,const std::string& _ip){ return m_Socket.send(_packet,_ip,m_Port); }
		sf::Socket::Status _send(const void* _data, size_t _size,const std::string& _ip){ return m_Socket.send(_data,_size,_ip,m_Port); }
		sf::Socket::Status _receive(sf::Packet& _packet,const std::string& _ip){ return m_Socket.receive(_packet,sf::IpAddress(_ip),m_Port); }
		sf::Socket::Status _receive(void* _data, size_t _size,size_t& _received,const std::string& _ip){ return m_Socket.receive(_data,_size,_received,sf::IpAddress(_ip),m_Port); }
};
SocketUDP::SocketUDP(const uint _port):m_i(new impl){
	m_i->_init(_port);
}
SocketUDP::~SocketUDP(){ unbind(); }
const sf::UdpSocket& SocketUDP::socket(){ return m_i->m_Socket; }
const ushort SocketUDP::localPort(){ return m_i->m_Socket.getLocalPort(); }
void SocketUDP::setBlocking(bool b){ m_i->m_Socket.setBlocking(b); }
const bool SocketUDP::isBlocking(){ return m_i->m_Socket.isBlocking(); }
void SocketUDP::bind(const std::string& _ip){
	sf::Socket::Status status = m_i->_bind(_ip);
}
void SocketUDP::unbind(){ m_i->_unbind(); }
void SocketUDP::send(sf::Packet& _packet,const std::string& _ip){
	sf::Socket::Status status = m_i->_send(_packet,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketUDP::send(const void* _data, size_t _size,const std::string& _ip){
	sf::Socket::Status status = m_i->_send(_data,_size,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketUDP::receive(sf::Packet& _packet,const std::string& _ip){
	sf::Socket::Status status = m_i->_receive(_packet,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}
void SocketUDP::receive(void* _data, size_t _size,size_t& _received,const std::string& _ip){
	sf::Socket::Status status = m_i->_receive(_data,_size,_received,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}

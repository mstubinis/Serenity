#include "Engine_Networking.h"

class Engine::Networking::SocketTCP::impl{
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
Engine::Networking::SocketTCP::SocketTCP(const uint _port,const std::string& _ip,uint _timeout):m_i(new impl){
	m_i->_init(_ip,_port,_timeout);
}
Engine::Networking::SocketTCP::~SocketTCP(){ disconnect(); }
const sf::TcpSocket& Engine::Networking::SocketTCP::socket(){ return m_i->m_Socket; }
const std::string& Engine::Networking::SocketTCP::ip(){ return m_i->m_Socket.getRemoteAddress().toString(); }
const ushort Engine::Networking::SocketTCP::remotePort(){ return m_i->m_Socket.getRemotePort(); }
const ushort Engine::Networking::SocketTCP::localPort(){ return m_i->m_Socket.getLocalPort(); }
void Engine::Networking::SocketTCP::connect(){
	sf::Socket::Status status = m_i->_connect();
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketTCP::disconnect(){ m_i->_disconnect(); }
void Engine::Networking::SocketTCP::setBlocking(bool b){ m_i->m_Socket.setBlocking(b); }
const bool Engine::Networking::SocketTCP::isBlocking(){ return m_i->m_Socket.isBlocking(); }
void Engine::Networking::SocketTCP::send(sf::Packet& _packet){
	sf::Socket::Status status = m_i->_send(_packet);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketTCP::send(const void* _data, size_t _size){
	sf::Socket::Status status = m_i->_send(_data,_size);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketTCP::send(const void* _data, size_t _size,size_t& _sent){
	sf::Socket::Status status = m_i->_send(_data,_size,_sent);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketTCP::receive(sf::Packet& _packet){
	sf::Socket::Status status = m_i->_receive(_packet);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketTCP::receive(void* _data, size_t _size,size_t& _sent){
	sf::Socket::Status status = m_i->_receive(_data,_size,_sent);
	if(status != sf::Socket::Done){
		//error
	}
}
class Engine::Networking::SocketUDP::impl{
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
Engine::Networking::SocketUDP::SocketUDP(const uint _port):m_i(new impl){
	m_i->_init(_port);
}
Engine::Networking::SocketUDP::~SocketUDP(){ unbind(); }
const sf::UdpSocket& Engine::Networking::SocketUDP::socket(){ return m_i->m_Socket; }
const ushort Engine::Networking::SocketUDP::localPort(){ return m_i->m_Socket.getLocalPort(); }
void Engine::Networking::SocketUDP::setBlocking(bool b){ m_i->m_Socket.setBlocking(b); }
const bool Engine::Networking::SocketUDP::isBlocking(){ return m_i->m_Socket.isBlocking(); }
void Engine::Networking::SocketUDP::bind(const std::string& _ip){
	sf::Socket::Status status = m_i->_bind(_ip);
}
void Engine::Networking::SocketUDP::unbind(){ m_i->_unbind(); }
void Engine::Networking::SocketUDP::send(sf::Packet& _packet,const std::string& _ip){
	sf::Socket::Status status = m_i->_send(_packet,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketUDP::send(const void* _data, size_t _size,const std::string& _ip){
	sf::Socket::Status status = m_i->_send(_data,_size,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketUDP::receive(sf::Packet& _packet,const std::string& _ip){
	sf::Socket::Status status = m_i->_receive(_packet,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::SocketUDP::receive(void* _data, size_t _size,size_t& _received,const std::string& _ip){
	sf::Socket::Status status = m_i->_receive(_data,_size,_received,_ip);
	if(status != sf::Socket::Done){
		//error
	}
}


class Engine::Networking::ListenerTCP::impl{
	public:
		sf::TcpListener m_Listener;
		std::string m_IP;
		ushort m_Port;
		void _init(const std::string& _ip, const uint _port){
			m_IP = _ip;
			m_Port = _port;
		}
		sf::Socket::Status _listen(){
			m_Listener.close();
			if(m_IP == ""){ return m_Listener.listen(m_Port,sf::IpAddress::Any); }
			else{ return m_Listener.listen(m_Port,m_IP); }
		}
		sf::Socket::Status _accept(sf::TcpSocket& _socket){
			return m_Listener.accept(_socket);
		}
};

Engine::Networking::ListenerTCP::ListenerTCP(const uint _port,const std::string& _ip):m_i(new impl){
	m_i->_init(_ip,_port);
}
Engine::Networking::ListenerTCP::~ListenerTCP(){ close(); }

const sf::TcpListener& Engine::Networking::ListenerTCP::socket(){
	return m_i->m_Listener;
}
const ushort Engine::Networking::ListenerTCP::localPort(){
	return m_i->m_Listener.getLocalPort();
}
void Engine::Networking::ListenerTCP::setBlocking(bool b){
	m_i->m_Listener.setBlocking(b);
}
const bool Engine::Networking::ListenerTCP::isBlocking(){
	return m_i->m_Listener.isBlocking();
}
void Engine::Networking::ListenerTCP::close(){
	m_i->m_Listener.close();
}
void Engine::Networking::ListenerTCP::accept(SocketTCP& _client){
	sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client.socket());
	sf::Socket::Status status = m_i->_accept(s);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::ListenerTCP::accept(SocketTCP* _client){
	sf::TcpSocket& s = const_cast<sf::TcpSocket&>(_client->socket());
	sf::Socket::Status status = m_i->_accept(s);
	if(status != sf::Socket::Done){
		//error
	}
}
void Engine::Networking::ListenerTCP::listen(){
	sf::Socket::Status status = m_i->_listen();
	if(status != sf::Socket::Done){
		//error
	}
}
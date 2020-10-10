#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/server/Server.h>
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/networking/SocketUDP.h>
#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/threading/ThreadingModule.h>

using namespace Engine;
using namespace Engine::Networking;

Server::Server(ServerType type, bool multithreaded) 
    : m_Clients{ Engine::hardware_concurrency() }
    , m_ServerType{ type }
    , m_Multithreaded{ multithreaded }
{}
Server::~Server() {
    shutdown();
}
ServerClient* Server::getClientFromUDPData(const std::string& ip, unsigned short port, sf::Packet& sf_packet) const {
    auto hash = m_Client_Hash_Function(ip, port, sf_packet);
    return m_Clients.getClient(hash);
}
bool Server::startup(unsigned short port, std::string ip_restriction) {
    if (m_Active.load(std::memory_order_relaxed) == true) {
        return false;
    }
    switch (m_ServerType) {
        case ServerType::TCP: {
            if (!m_TCPListener) {
                m_TCPListener = make_unique<ListenerTCP>(port, ip_restriction);
            }
            break;
        }case ServerType::UDP: {
            if (!m_UdpSocket) {
                m_UdpSocket   = make_unique<SocketUDP>(port, ip_restriction);
            }
            break;
        }
    }
    if (m_TCPListener) {
        m_TCPListener->setBlocking(false);
        auto status  = m_TCPListener->listen();
        if (status != SocketStatus::Done) {
            ENGINE_PRODUCTION_LOG("Server::startup() : could not start up TCP Listener")
        }
    }
    if (m_UdpSocket) {
        m_UdpSocket->setBlocking(false);
        auto status  = m_UdpSocket->bind();
        if (status != SocketStatus::Done) {
            ENGINE_PRODUCTION_LOG("Server::startup() : could not start up UDP Socket")
        }
    }

    m_Port           = port;
    m_IP_Restriction = ip_restriction;
    m_Active.store(true, std::memory_order_relaxed);

    registerEvent(EventType::ClientConnected);
    registerEvent(EventType::ClientDisconnected);
    registerEvent(EventType::ServerStarted);
    registerEvent(EventType::ServerShutdowned);
    registerEvent(EventType::PacketSent);
    registerEvent(EventType::PacketReceived);

    return true;
}
bool Server::shutdown() {
    if (m_Active.load(std::memory_order_relaxed) == false) {
        return false;
    }
    unregisterEvent(EventType::ClientConnected);
    unregisterEvent(EventType::ClientDisconnected);
    unregisterEvent(EventType::ServerStarted);
    unregisterEvent(EventType::ServerShutdowned);
    unregisterEvent(EventType::PacketSent);
    unregisterEvent(EventType::PacketReceived);

    if (m_UdpSocket) {
        m_UdpSocket->unbind();
    }
    if (m_TCPListener) {
        m_TCPListener->close();
    }
    m_Active.store(false, std::memory_order_relaxed);
    return true;
}
void Server::clearAllClients() {
    m_Clients.clearAllClients();
}
void Server::internal_send_to_all_tcp(const ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& itr : m_Clients) {
        if (itr.second != exclusion) {
            auto status = itr.second->send_tcp(sf_packet);
        }
    }
}

void Server::internal_send_to_all_udp(const ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& itr : m_Clients) {
        if (itr.second != exclusion) {
            auto status = itr.second->send_udp(sf_packet);
        }
    }
}

SocketStatus::Status Server::send_tcp_to_client(ServerClient* client, sf::Packet& sf_packet) {
    return client->send_tcp(sf_packet);
}
void Server::send_tcp_to_all_but_client(const ServerClient* exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_tcp(exclusion, sf_packet);
}
void Server::send_tcp_to_all(sf::Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}

SocketStatus::Status Server::send_udp_to_client(ServerClient* client, sf::Packet& sf_packet) {
    return m_UdpSocket->send(client->port(), sf_packet, client->ip().toString());
}
void Server::send_udp_to_all_but_client(const ServerClient* exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_udp(exclusion, sf_packet);
}
void Server::send_udp_to_all(sf::Packet& sf_packet) {
    internal_send_to_all_udp(nullptr, sf_packet);
}

SocketStatus::Status Server::send_udp_to_client_important(ServerClient* client, Engine::Networking::Packet& packet) {
    return Server::send_udp_to_client(client, packet);
}
void Server::send_udp_to_all_but_client_important(const ServerClient* exclusion, Engine::Networking::Packet& packet) {
    Server::send_udp_to_all_but_client(exclusion, packet);
}
void Server::send_udp_to_all_important(Engine::Networking::Packet& packet) {
    Server::send_udp_to_all(packet);
}

SocketStatus::Status Server::receive_udp(sf::Packet& sf_packet, sf::IpAddress& sender, unsigned short& port) {
    return m_UdpSocket->receive(sf_packet, sender, port);
}
bool Server::internal_add_client(const std::string& hash, ServerClient* client) {
    if (m_Clients.contains(hash)) {
        SAFE_DELETE(client);
        return false;
    }
    if (!client) {
        ENGINE_PRODUCTION_LOG("Server::internal_add_client() called with a nullptr client")
        return false;
    }
    bool result = m_Clients.addClient(hash, client);
    if(result){
        ENGINE_PRODUCTION_LOG("Server::internal_add_client() accepted new client: " << client->ip() << " on port: " << client->port())
    }else{
        ENGINE_PRODUCTION_LOG("Server::internal_add_client() REJECTED new client: " << client->ip() << " on port: " << client->port() << " due to not finding a next_thread")
        SAFE_DELETE(client);
    }
    return result;
}
void Server::remove_client_immediately(ServerClient& inClient) {
    m_Clients.removeClientImmediately(inClient, m_Mutex);
}
void Server::remove_client(ServerClient& inClient) {
    m_Clients.removeClient(inClient, m_Mutex);
}
ServerClient* Server::add_new_client(std::string& hash, std::string& clientIP, unsigned short clientPort, SocketTCP* tcp) {
    ServerClient* client = NEW ServerClient(hash, *this, tcp, clientIP, clientPort);
    return client;
}

void Server::internal_update_tcp_listener_loop(bool serverActive) {
    if (!serverActive) {
        return;
    }
    if (m_TCPListener) {
        auto* new_tcp_socket = NEW SocketTCP();
        auto status          = m_TCPListener->accept(*new_tcp_socket);
        if (status == sf::Socket::Status::Done) {
            sf::Packet dummy;
            auto client_ip   = new_tcp_socket->ip();
            auto client_port = new_tcp_socket->remotePort();
            auto client_hash = m_Client_Hash_Function(client_ip, client_port, dummy);
            internal_add_client(client_hash, add_new_client(client_hash, client_ip, client_port, new_tcp_socket));
        }else{
            SAFE_DELETE(new_tcp_socket);
        }
    }
}

void Server::onReceiveUDP(Engine::Networking::Packet& packet, sf::IpAddress& ip, unsigned short port, const float dt) {
    std::string ipAsString  = ip.toString();
    std::string client_hash = m_Client_Hash_Function(ipAsString, port, packet);
    if (m_ServerType == ServerType::UDP) {
        internal_add_client(client_hash, add_new_client(client_hash, ipAsString, port, nullptr));
    }
    m_Clients.getClient(client_hash)->internal_on_receive_udp(packet, dt);
    Server::m_On_Receive_UDP_Function(packet, ipAsString, port, dt);
}
void Server::internal_update_udp_loop(const float dt, bool serverActive) {
    if (!serverActive) {
        return;
    }
    Engine::Networking::Packet packet;
    sf::IpAddress              ip;
    unsigned short             port;

    auto status = receive_udp(packet, ip, port);

    switch (status) {
        case sf::Socket::Status::Done: {
            onReceiveUDP(packet, ip, port, dt);
            break;
        }case sf::Socket::Status::NotReady: {
            break;
        }case sf::Socket::Status::Partial: {
            break;
        }case sf::Socket::Status::Disconnected: {
            break;
        }case sf::Socket::Status::Error: {
            break;
        }
    }
}
void Server::internal_update_client_threads(const float dt, bool serverActive) {
    if (!serverActive) {
        return;
    }
    auto lambda_update_client_thread = [this , serverActive](ServerThread& thread, const float dt) {
        for (auto& [hash, client] : thread.clients()) {
            if (serverActive == false ||  thread.num_clients() == 0) { //can be inactive if no clients are using the thread
                return true;
            }
            if (client) {
                client->update(dt);
            }
        }
        return false;
    };
    if (m_Multithreaded) {
        //TODO: it works but it's kind of hacky
        for (auto& clientThread : m_Clients.getThreads()) {
            auto lambda_update_client_thread_driver = [&lambda_update_client_thread, &clientThread, dt]() {
                return lambda_update_client_thread(clientThread, dt);
            };
            Engine::priv::threading::addJob(std::move(lambda_update_client_thread_driver));
        }
    }else{
        for (auto& clientThread : m_Clients.getThreads()) {
            lambda_update_client_thread(clientThread, dt);
        }
    }
}
void Server::update(const float dt) {
    auto serverActive = m_Active.load(std::memory_order_relaxed);

    internal_update_tcp_listener_loop(serverActive);
    internal_update_udp_loop(dt, serverActive);
    internal_update_client_threads(dt, serverActive);

    m_Update_Function(dt, serverActive);

    m_Clients.internal_update_remove_clients();
}

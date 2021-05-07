
#include <serenity/networking/server/Server.h>
#include <serenity/networking/server/ServerClient.h>
#include <serenity/networking/SocketTCP.h>
#include <serenity/networking/SocketUDP.h>
#include <serenity/networking/ListenerTCP.h>
#include <serenity/threading/ThreadingModule.h>

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
ServerClient* Server::getServerClientFromUDPData(std::string_view clientIP, uint16_t clientPort) const {
    return m_Clients.getClient( m_CustomClientHashFunction(std::string{ clientIP }, clientPort) );
}
bool Server::startup(uint16_t port, std::string ip_restriction) {
    if (isActive()) {
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
            ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): could not start up TCP Listener")
        }
    }
    if (m_UdpSocket) {
        m_UdpSocket->setBlocking(false);
        auto status  = m_UdpSocket->bind();
        if (status != SocketStatus::Done) {
            ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): could not start up UDP Socket")
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
    if (!isActive()) {
        return false;
    }
    unregisterEventImmediate(EventType::ClientConnected);
    unregisterEventImmediate(EventType::ClientDisconnected);
    unregisterEventImmediate(EventType::ServerStarted);
    unregisterEventImmediate(EventType::ServerShutdowned);
    unregisterEventImmediate(EventType::PacketSent);
    unregisterEventImmediate(EventType::PacketReceived);
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
bool Server::internal_add_client(const std::string& hash, ServerClient* client) {
    if (m_Clients.contains(hash)) {
        SAFE_DELETE(client);
        return false;
    }
    if (!client) {
        return false;
    }
    bool result = m_Clients.addClient(hash, client);
    if(result){
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): accepted new client: " << client->ip() << " on port: " << client->port())
    }else{
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): REJECTED new client: " << client->ip() << " on port: " << client->port() << " due to not finding a next_thread")
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
ServerClient* Server::add_new_client(const std::string& hash, sf::IpAddress clientIP, uint16_t clientPort, SocketTCP* tcp) {
    return NEW ServerClient{ hash, *this, tcp, std::move(clientIP), clientPort };
}
void Server::internal_on_receive_udp_packet(Engine::Networking::Packet& packet, sf::IpAddress ip, uint16_t port) {
    std::string ipAsString  = ip.toString();
    std::string client_hash = m_CustomClientHashFunction(ipAsString, port);
    if (m_ServerType == ServerType::UDP) {
        internal_add_client( client_hash, add_new_client(client_hash, ipAsString, port, nullptr) );
    }
    m_Clients.getClient(client_hash)->internal_on_receive_udp(packet);
    Server::m_CustomOnReceiveUDPFunction(packet, ipAsString, port);
}
void Server::update(const float dt) {
    auto serverActive = isActive();
    m_CustomUpdateFunction(dt, serverActive);
    m_Clients.internal_update_remove_clients();
}
std::pair<Engine::Networking::Packet, SocketStatus::Status> Server::onUpdateReceiveUDPPackets() {
    const auto serverActive = isActive();
    Engine::Networking::Packet packet;
    if (!serverActive) {
        return { packet, SocketStatus::Error };
    }
    sf::IpAddress   ipSender;
    uint16_t        portSender;
    auto status = m_UdpSocket->receive(packet, ipSender, portSender);
    switch (status) {
        case SocketStatus::Done: {
            internal_on_receive_udp_packet(packet, ipSender, portSender);
            break;
        } case SocketStatus::NotReady: {
            break;
        } case SocketStatus::Partial: {
            break;
        } case SocketStatus::Disconnected: {
            break;
        } case SocketStatus::Error: {
            break;
        }
    }
    return {packet, status};
}
void Server::onUpdateReceiveTCPPackets(const float dt) {
    const auto serverActive = isActive();
    if (!serverActive) {
        return;
    }
    auto lambda_update_client_thread = [this, serverActive](ServerThread& thread, const float dt) {
        for (auto& [hash, client] : thread.clients()) {
            if (serverActive == false || thread.num_clients() == 0) { //can be inactive if no clients are using the thread
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
    } else {
        for (auto& clientThread : m_Clients.getThreads()) {
            lambda_update_client_thread(clientThread, dt);
        }
    }
}
void Server::onUpdateProcessTCPListeners() {
    const auto serverActive = isActive();
    if (!serverActive) {
        return;
    }
    if (m_TCPListener) {
        SocketTCP* tcpSocket = NEW SocketTCP();
        auto status = m_TCPListener->accept(*tcpSocket);
        if (status == sf::Socket::Status::Done) {
            auto client_ip   = tcpSocket->ip();
            auto client_port = tcpSocket->remotePort();
            auto client_hash = m_CustomClientHashFunction(client_ip, client_port);
            internal_add_client( client_hash, add_new_client(client_hash, client_ip, client_port, tcpSocket) );
        } else {
            SAFE_DELETE(tcpSocket);
        }
    }
}
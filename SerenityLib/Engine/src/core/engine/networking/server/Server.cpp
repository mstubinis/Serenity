#include <core/engine/networking/server/Server.h>
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/networking/SocketUDP.h>
#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/threading/ThreadingModule.h>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

Server::Server(ServerType::Type type, bool multithreaded) : m_Threads(Engine::hardware_concurrency()){
    m_ServerType    = type;
    m_Multithreaded = multithreaded;
}
Server::~Server() {
    shutdown();
}

ServerClient* Server::getClientFromUDPData(const string& ip, unsigned short port, sf::Packet& sf_packet) const {
    auto hash = m_Client_Hash_Function(ip, port, sf_packet);
    if (m_HashedClients.count(hash)) {
        return m_HashedClients.at(hash);
    }
    return nullptr;
}
bool Server::startup(unsigned short port, string ip_restriction) {
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
        }default: {
            break;
        }
    }
    if (m_TCPListener) {
        m_TCPListener->setBlocking(false);
        auto status  = m_TCPListener->listen();
        //TODO: print error message if status != Done
    }
    if (m_UdpSocket) {
        m_UdpSocket->setBlocking(false);
        auto status  = m_UdpSocket->bind();
        //TODO: print error message if status != Done
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

    m_UdpSocket->unbind();
    if (m_TCPListener) {
        m_TCPListener->close();
    }
    m_Active.store(false, std::memory_order_relaxed);
    return true;
}

void Server::internal_send_to_all_tcp(ServerClient* exclusion, Packet& packet) {
    for (auto& itr : m_HashedClients) {
        if (itr.second != exclusion) {
            auto status = itr.second->send_tcp(packet);
        }
    }
}
void Server::internal_send_to_all_udp(ServerClient* exclusion, Packet& packet) {
    for (auto& itr : m_HashedClients) {
        if (itr.second != exclusion) {
            auto clientIP   = itr.second->ip();
            auto clientPort = itr.second->port();
            auto status     = m_UdpSocket->send(clientPort, packet, clientIP);
        }
    }
}
void Server::internal_send_to_all_tcp(ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& itr : m_HashedClients) {
        if (itr.second != exclusion) {
            auto status = itr.second->send_tcp(sf_packet);
        }
    }
}
void Server::internal_send_to_all_udp(ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& itr : m_HashedClients) {
        if (itr.second != exclusion) {
            auto clientIP   = itr.second->ip();
            auto clientPort = itr.second->port();
            auto status     = m_UdpSocket->send(clientPort, sf_packet, clientIP);
        }
    }
}

SocketStatus::Status Server::send_tcp_to_client(ServerClient& client, Packet& packet) {
    return client.send_tcp(packet);
}
SocketStatus::Status Server::send_tcp_to_client(ServerClient& client, sf::Packet& sf_packet) {
    return client.send_tcp(sf_packet);
}

void Server::send_tcp_to_all_but_client(ServerClient& exclusion, Packet& packet) {
    internal_send_to_all_tcp(&exclusion, packet);
}
void Server::send_tcp_to_all_but_client(ServerClient& exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_tcp(&exclusion, sf_packet);
}

void Server::send_tcp_to_all(Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}
void Server::send_tcp_to_all(sf::Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}

SocketStatus::Status Server::send_udp_to_client(ServerClient& client, Packet& packet) {
    return m_UdpSocket->send(client.port(), packet, client.ip());
}
SocketStatus::Status Server::send_udp_to_client(ServerClient& client, sf::Packet& sf_packet) {
    return m_UdpSocket->send(client.port(), sf_packet, client.ip());
}

void Server::send_udp_to_all_but_client(ServerClient& exclusion, Packet& packet) {
    internal_send_to_all_udp(&exclusion, packet);
}
void Server::send_udp_to_all_but_client(ServerClient& exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_udp(&exclusion, sf_packet);
}

void Server::send_udp_to_all(Packet& packet) {
    internal_send_to_all_udp(nullptr, packet);
}
void Server::send_udp_to_all(sf::Packet& sf_packet) {
    internal_send_to_all_udp(nullptr, sf_packet);
}

SocketStatus::Status Server::receive_udp(sf::Packet& sf_packet, sf::IpAddress& sender, unsigned short& port) {
    return m_UdpSocket->receive(sf_packet, sender, port);
}

void Server::internal_add_client(string& hash, ServerClient& client) {
    bool result = m_Threads.addClient(hash, &client, *this);
    if(result){
        std::cout << "Server: accepted new client: " << client.ip() << " on port: " << client.port() << "\n";
    }else{
        std::cout << "Server: REJECTED new client: " << client.ip() << " on port: " << client.port() << " due to not finding a next_thread\n";
    }
}
void Server::remove_client(ServerClient& client) {
    for (auto& itr : m_HashedClients) {
        if (itr.second == &client) {
            std::cout << "Client: " << itr.second->ip() << " - has been completely removed from the server" << std::endl;
            {
                std::lock_guard lock(m_Mutex);
                m_RemovedClients.push_back(make_pair(itr.first, itr.second));
            }
            return;
        }
    }
}
ServerClient* Server::add_new_client(string& hash, string& clientIP, unsigned short clientPort, SocketTCP* tcp) {
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
            auto client_ip   = new_tcp_socket->ip();
            auto client_port = new_tcp_socket->remotePort();
            sf::Packet dummy;
            auto client_hash = m_Client_Hash_Function(client_ip, client_port, dummy);
            auto* new_client = add_new_client(client_hash, client_ip, client_port, new_tcp_socket);
            if (new_client) {
                internal_add_client(client_hash, *new_client);
            }
        }else{
            SAFE_DELETE(new_tcp_socket);
        }
    }
}
void Server::internal_update_udp_loop(const float dt, bool serverActive) {
    if (!serverActive) {
        return;
    }
    sf::Packet      sf_packet;
    sf::IpAddress   ip;
    unsigned short  port;
    auto status = receive_udp(sf_packet, ip, port);

    switch (status) {
        case sf::Socket::Status::Done: {
            string ipAsString = ip.toString();
            if (m_ServerType == ServerType::UDP) {
                auto client_hash = m_Client_Hash_Function(ipAsString, port, sf_packet);
                if (!m_HashedClients.count(client_hash)) {
                    auto* new_client = add_new_client(client_hash, ipAsString, port, nullptr);
                    if (new_client) {
                        internal_add_client(client_hash, *new_client);
                    }
                }
            }
            m_On_Receive_UDP_Function(sf_packet, ipAsString, port, dt);
            break;
        }case sf::Socket::Status::NotReady: {
            break;
        }case sf::Socket::Status::Partial: {
            break;
        }case sf::Socket::Status::Disconnected: {
            break;
        }case sf::Socket::Status::Error: {
            break;
        }default: {
            break;
        }
    }
}
void Server::internal_update_client_threads(const float dt, bool serverActive) {
    if (!serverActive) {
        return;
    }
    auto lambda_update_client_thread = [this , serverActive  ](ServerThread& thread, const float dt) {
        for (auto& client_itr : thread.clients()) {
            if (serverActive == false ||  thread.num_clients() == 0) { //can be inactive if no clients are using the thread
                return true;
            }
            if (client_itr.second) {
                client_itr.second->update(dt);
            }
        }
        return false;
    };
    if (m_Multithreaded) {
        //TODO: it works but it's kind of hacky
        for (auto& clientThread : m_Threads) {
            auto lambda_update_client_thread_driver = [&lambda_update_client_thread, &clientThread, dt]() {
                return lambda_update_client_thread(clientThread, dt);
            };
            Engine::priv::threading::addJob(std::move(lambda_update_client_thread_driver));
        }
    }else{
        for (auto& clientThread : m_Threads) {
            lambda_update_client_thread(clientThread, dt);
        }
    }
}
void Server::internal_update_remove_clients() {
    if (m_RemovedClients.size() > 0) {
        Engine::priv::threading::waitForAll();
        for (const auto& itr : m_RemovedClients) {
            bool result = m_Threads.removeClient(itr.first, *this);
        }
        m_RemovedClients.clear();
    }
}
void Server::update(const float dt) {
    auto serverActive = m_Active.load(std::memory_order_relaxed);

    internal_update_tcp_listener_loop(serverActive);
    internal_update_udp_loop(dt, serverActive);
    internal_update_client_threads(dt, serverActive);

    m_Update_Function(dt, serverActive);

    internal_update_remove_clients();
}


/*
void Server::internal_send_to_all_tcp(ServerClient* exclusion, void* data, size_t size) {
    for (auto& itr : m_HashedClients) {
        if (itr.second != exclusion) {
            auto status = itr.second->send_tcp(data, size);
        }
    }
}
void Server::internal_send_to_all_udp(ServerClient* exclusion, void* data, size_t size) {
    for (auto& itr : m_HashedClients) {
        if (itr.second != exclusion) {
            const auto clientIP = itr.second->ip();
            const auto clientID = itr.second->id();
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, data, size, clientIP);
        }
    }
}
void Server::send_tcp_to_all_but_client(ServerClient& exclusion, void* data, size_t size) {
    internal_send_to_all_tcp(&exclusion, data, size);
}
void Server::send_tcp_to_all(void* data, size_t size) {
    internal_send_to_all_tcp(nullptr, data, size);
}
void Server::send_udp_to_all_but_client(ServerClient& exclusion, void* data, size_t size) {
    internal_send_to_all_udp(&exclusion, data, size);
}
SocketStatus::Status Server::send_tcp_to_client(ServerClient& client, void* data, size_t size) {
    return client.send_tcp(data, size);
}
SocketStatus::Status Server::send_tcp_to_client(ServerClient& client, void* data, size_t size, size_t& sent) {
    return client.send_tcp(data, size, sent);
}
void Server::send_udp_to_all(void* data, size_t size) {
    internal_send_to_all_udp(nullptr, data, size);
}
SocketStatus::Status Server::receive_udp(void* data, size_t size, size_t& received, sf::IpAddress& sender, unsigned short& port) {
    return m_UdpSocket->receive(data, size, received, sender, port);
}

void Server::setBlockingTCPListener(bool blocking) {
    m_TCPListener->setBlocking(blocking);
}
void Server::setBlockingTCPClients(bool blocking) {
    m_Threads.setBlocking(blocking);
}
void Server::setBlockingTCPClient(string& hash, bool blocking) {
    m_Threads.setBlocking(hash, blocking);
}
void Server::setBlockingUDP(bool blocking) {
    m_UdpSocket->setBlocking(blocking);
}

*/
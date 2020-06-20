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

Engine::Networking::Server::Server(ServerType::Type type) {
    m_ServerType = type;
    auto hardware_concurrency = Engine::hardware_concurrency();
    for (size_t i = 0; i < hardware_concurrency; ++i) {
        m_Threads.emplace_back();
    }
}
Engine::Networking::Server::~Server() {
    shutdown();
}
ServerType::Type Engine::Networking::Server::getType() const {
    return m_ServerType;
}
unsigned int Engine::Networking::Server::num_clients() const {
    unsigned int number_of_clients = 0;
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        number_of_clients += m_Threads[i].num_clients();
    }
    return number_of_clients;
}
Engine::Networking::ServerThread* Engine::Networking::Server::getNextAvailableClientThread() {
    Engine::Networking::ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = (&clientThread);
        }
    }
    return leastThread;
}
vector<Engine::Networking::ServerClient*> Engine::Networking::Server::clients() const {
    vector<Engine::Networking::ServerClient*> ret;
    ret.reserve(num_clients());
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].clients()) {
            if (client.second) {
                ret.push_back(client.second);
            }
        }
    }
    return ret;
}
void Engine::Networking::Server::setClientHashFunction(function<string(string ip, unsigned short port, std::string extra)> function) {
    m_Client_Hash_Function = function;
}
void Engine::Networking::Server::setServerUpdateFunction(std::function<void(const float dt, bool serverActive)> function) {
    m_Update_Function = function;
}
void Engine::Networking::Server::setExtraHashExtractUDPFunction(std::function<std::string(sf::Packet& packet)> function) {
    m_Extract_Extra_UDP_Function = function;
}
void Engine::Networking::Server::setBlockingTCPListener(bool blocking) {
    m_TCPListener->setBlocking(blocking);
}
void Engine::Networking::Server::setBlockingTCPClients(bool blocking) {
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].clients()) {
            client.second->socket()->setBlocking(blocking);
        }
    }
}
void Engine::Networking::Server::setBlockingTCPClient(string& client_hash, bool blocking) {
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].clients()) {
            if (client.first == client_hash) {
                client.second->socket()->setBlocking(blocking);
                return;
            }
        }
    }
}
void Engine::Networking::Server::setBlockingUDP(bool blocking) {
    m_UdpSocket->setBlocking(blocking);
}

bool Engine::Networking::Server::startup(unsigned short port, string ip_restriction) {
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
        }case ServerType::TCP_AND_UDP: {
            if (!m_TCPListener) {
                m_TCPListener = make_unique<ListenerTCP>(port, ip_restriction);
            }
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
    }
    if (m_UdpSocket) {
        m_UdpSocket->setBlocking(false);
        auto status  = m_UdpSocket->bind();
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
bool Engine::Networking::Server::shutdown() {
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
    m_TCPListener->close();

    m_Active.store(false, std::memory_order_relaxed);
    return true;
}
ServerThread* Engine::Networking::Server::internal_get_next_available_thread() {
    ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = &clientThread;
        }
    }
    return leastThread;
}

void Engine::Networking::Server::internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, Engine::Networking::Packet& packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            auto status = client->send_tcp(packet);
        }
    }
}
void Engine::Networking::Server::internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, Engine::Networking::Packet& packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            const auto clientIP = client->ip();
            const auto clientID = client->id();
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, packet, clientIP);
        }
    }
}

void Engine::Networking::Server::internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            auto status = client->send_tcp(sf_packet);
        }
    }
}
void Engine::Networking::Server::internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            const auto clientIP = client->ip();
            const auto clientID = client->id();
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, sf_packet, clientIP);
        }
    }
}
void Engine::Networking::Server::internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, void* data, size_t size) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            auto status = client->send_tcp(data, size);
        }
    }
}
void Engine::Networking::Server::internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, void* data, size_t size) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread.clients()) {
            if (client.second != exclusion) {
                const auto clientIP = client.second->ip();
                const auto clientID = client.second->id();
                auto status = m_UdpSocket->send(m_Port + 1 + clientID, data, size, clientIP);
            }
        }
    }
}
SocketStatus::Status Engine::Networking::Server::send_tcp_to_client(Engine::Networking::ServerClient& client, Engine::Networking::Packet& packet) {
    return client.send_tcp(packet);
}
SocketStatus::Status Engine::Networking::Server::send_tcp_to_client(Engine::Networking::ServerClient& client, sf::Packet& sf_packet) {
    return client.send_tcp(sf_packet);
}
SocketStatus::Status Engine::Networking::Server::send_tcp_to_client(Engine::Networking::ServerClient& client, void* data, size_t size) {
    return client.send_tcp(data, size);
}
SocketStatus::Status Engine::Networking::Server::send_tcp_to_client(Engine::Networking::ServerClient& client, void* data, size_t size, size_t& sent) {
    return client.send_tcp(data, size, sent);
}
void Engine::Networking::Server::send_tcp_to_all_but_client(Engine::Networking::ServerClient& exclusion, Engine::Networking::Packet& packet) {
    internal_send_to_all_tcp(&exclusion, packet);
}
void Engine::Networking::Server::send_tcp_to_all_but_client(Engine::Networking::ServerClient& exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_tcp(&exclusion, sf_packet);
}
void Engine::Networking::Server::send_tcp_to_all_but_client(Engine::Networking::ServerClient& exclusion, void* data, size_t size) {
    internal_send_to_all_tcp(&exclusion, data, size);
}
void Engine::Networking::Server::send_tcp_to_all(Engine::Networking::Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}
void Engine::Networking::Server::send_tcp_to_all(sf::Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}
void Engine::Networking::Server::send_tcp_to_all(void* data, size_t size) {
    internal_send_to_all_tcp(nullptr, data, size);
}
void Engine::Networking::Server::send_udp_to_all_but_client(Engine::Networking::ServerClient& exclusion, Engine::Networking::Packet& packet) {
    internal_send_to_all_udp(&exclusion, packet);
}
void Engine::Networking::Server::send_udp_to_all_but_client(Engine::Networking::ServerClient& exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_udp(&exclusion, sf_packet);
}
void Engine::Networking::Server::send_udp_to_all_but_client(Engine::Networking::ServerClient& exclusion, void* data, size_t size) {
    internal_send_to_all_udp(&exclusion, data, size);
}
void Engine::Networking::Server::send_udp_to_all(Engine::Networking::Packet& packet) {
    internal_send_to_all_udp(nullptr, packet);
}
void Engine::Networking::Server::send_udp_to_all(sf::Packet& sf_packet) {
    internal_send_to_all_udp(nullptr, sf_packet);
}
void Engine::Networking::Server::send_udp_to_all(void* data, size_t size) {
    internal_send_to_all_udp(nullptr, data, size);
}
SocketStatus::Status Engine::Networking::Server::receive_udp(sf::Packet& sf_packet, sf::IpAddress& sender, unsigned short& port) {
    return m_UdpSocket->receive(sf_packet, sender, port);
}
SocketStatus::Status Engine::Networking::Server::receive_udp(void* data, size_t size, size_t& received, sf::IpAddress& sender, unsigned short& port) {
    return m_UdpSocket->receive(data, size, received, sender, port);
}


void Engine::Networking::Server::internal_add_client(string& hash, Engine::Networking::ServerClient& client) {
    auto* next_thread = internal_get_next_available_thread();
    if (next_thread) {
        bool result = next_thread->add_client(hash, &client, *this);
        std::cout << "Server: TCP listener accepted new client: " << client.ip() << " on port: " << client.port() << "\n";
    }else{
        std::cout << "Server: TCP listener REJECTED new client: " << client.ip() << " on port: " << client.port() << " due to not finding a next_thread\n";
    }
}
void Engine::Networking::Server::remove_client(Engine::Networking::ServerClient& client) {
    for (auto& clientThread : m_Threads) {
        for (auto& client_itr : clientThread.clients()) {
            auto& c = *static_cast<ServerClient*>(client_itr.second);
            if (client_itr.second == &client) {
                std::cout << "Client: " << client_itr.second->ip() << " - has been completely removed from the server" << std::endl;
                {
                    std::lock_guard lock(m_Mutex);
                    m_RemovedClients.push_back(make_pair(client_itr.first, client_itr.second));
                }
                return;
            }
        }
    }
}

ServerClient* Engine::Networking::Server::add_new_client(string& hash, string& clientIP, unsigned short clientPort, Engine::Networking::SocketTCP* tcp) {
    ServerClient* client = NEW ServerClient(hash, *this, tcp, clientIP, clientPort);
    return client;
}
void Engine::Networking::Server::internal_update_tcp_listener_loop() {
    if (m_TCPListener) {
        auto* tcp_socket     = NEW Engine::Networking::SocketTCP();
        auto status          = m_TCPListener->accept(*tcp_socket);
        if (status == sf::Socket::Status::Done) {
            auto client_ip   = tcp_socket->ip();
            auto client_port = tcp_socket->remotePort();
            auto client_hash = m_Client_Hash_Function(client_ip, client_port, "");
            auto* new_client = add_new_client(client_hash, client_ip, client_port, tcp_socket);
            if (new_client) {
                internal_add_client(client_hash, *new_client);
            }
        }else{
            SAFE_DELETE(tcp_socket);
        }
    }
}
void Engine::Networking::Server::internal_update_loop(const float dt, bool serverActive) {
    if (!serverActive) {
        return;
    }
    internal_update_tcp_listener_loop();

    //server udp socket
    sf::Packet sf_packet_udp;
    sf::IpAddress ip;
    std::string ipAsString;
    unsigned short port;
    auto status_udp = receive_udp(sf_packet_udp, ip, port);
    if (status_udp == sf::Socket::Done) {
        on_recieve_udp(sf_packet_udp);
        //if the server is only of type udp, then we dont have a tcp listener and thus need a way to add clients
        if (m_ServerType == ServerType::UDP) {
            ipAsString = ip.toString();
            auto extraInfo   = m_Extract_Extra_UDP_Function(sf_packet_udp);
            auto client_hash = m_Client_Hash_Function(ipAsString, port, extraInfo);
            if (!m_HashedClients.count(client_hash)) {
                auto* new_client = add_new_client(client_hash, ipAsString, port, nullptr);
                if (new_client) {
                    internal_add_client(client_hash, *new_client);
                }
            }else{
                //forward udp data to proper client
                m_HashedClients.at(client_hash)->receive_udp(status_udp, sf_packet_udp, dt);
            }
        }
    }

    //TODO: it works but it's kind of hacky ///////////////////////////////////////////////////
    for (auto& clientThread : m_Threads) {
        auto lambda_update_client_thread = [dt, this, serverActive, &clientThread]() {
            for (auto& client_itr : clientThread.clients()) {
                if (serverActive == 0 || clientThread.num_clients() == 0) { //can be inactive if no clients are using the thread
                    return true;
                }
                if (client_itr.second) {
                    client_itr.second->internal_update_loop(dt);
                    client_itr.second->update(dt);
                }
            }
            return false;
        };
        Engine::priv::threading::addJob(std::move(lambda_update_client_thread));
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
}
void Engine::Networking::Server::update(const float dt) {
    auto serverActive = m_Active.load(std::memory_order_relaxed);
    internal_update_loop(dt, serverActive);
    m_Update_Function(dt, serverActive);

    //remove clients
    if (m_RemovedClients.size() > 0) {
        Engine::priv::threading::waitForAll();
        for (const auto& pair_itr : m_RemovedClients) {
            bool complete = false;
            for (auto& clientThread : m_Threads) {
                for (auto& client_itr : clientThread.clients()) {
                    if (client_itr.first == pair_itr.first) {
                        auto& hash = client_itr.first;
                        bool result = clientThread.remove_client(hash, *this);
                        complete = true;
                        break;
                    }
                }
                if (complete) {
                    break;
                }
            }
        }
        m_RemovedClients.clear();
    }
}
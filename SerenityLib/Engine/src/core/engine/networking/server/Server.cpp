#include <core/engine/networking/server/Server.h>
#include <core/engine/networking/server/ServerClient.h>
#include <core/engine/networking/SocketTCP.h>
#include <core/engine/networking/SocketUDP.h>
#include <core/engine/networking/ListenerTCP.h>
#include <core/engine/threading/Engine_ThreadManager.h>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

Server::Server(ServerType::Type type) {
    m_ServerType = type;
    auto hardware_concurrency = Engine::priv::threading::hardware_concurrency();
    for (size_t i = 0; i < hardware_concurrency; ++i) {
        m_Threads.emplace_back();
    }
}
Server::~Server() {
    shutdown();
}
unsigned int Server::num_clients() const {
    unsigned int number_of_clients = 0;
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        number_of_clients += m_Threads[i].num_clients();
    }
    return number_of_clients;
}
Engine::Networking::ServerThread* Server::getNextAvailableClientThread() {
    Engine::Networking::ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = (&clientThread);
        }
    }
    return leastThread;
}
vector<Engine::Networking::ServerClient*> Server::clients() const {
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
void Server::setClientHashFunction(function<string(string ip, unsigned short port)> function) {
    m_Client_Hash_Function = function;
}
void Server::setServerUpdateFunction(std::function<void(const float dt)> function) {
    m_Update_Function = function;
}
void Server::setBlockingTCPListener(bool blocking) {
    m_TCPListener->setBlocking(blocking);
}
void Server::setBlockingTCPClients(bool blocking) {
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].clients()) {
            client.second->socket()->setBlocking(blocking);
        }
    }
}
void Server::setBlockingTCPClient(string& client_hash, bool blocking) {
    for (size_t i = 0; i < m_Threads.size(); ++i) {
        for (const auto& client : m_Threads[i].clients()) {
            if (client.first == client_hash) {
                client.second->socket()->setBlocking(blocking);
                return;
            }
        }
    }
}
void Server::setBlockingUDP(bool blocking) {
    m_UdpSocket->setBlocking(blocking);
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
    m_TCPListener->close();

    m_Active.store(false, std::memory_order_relaxed);
    return true;
}
ServerThread* Server::internal_get_next_available_thread() {
    ServerThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->num_clients() < clientThread.num_clients())) {
            leastThread = &clientThread;
        }
    }
    return leastThread;
}

void Server::internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, Engine::Networking::Packet& packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            auto status = client->send(packet);
        }
    }
}
void Server::internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, Engine::Networking::Packet& packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            const auto clientIP = client->ip();
            const auto clientID = client->id();
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, packet, clientIP);
        }
    }
}

void Server::internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            auto status = client->send(sf_packet);
        }
    }
}
void Server::internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, sf::Packet& sf_packet) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            const auto clientIP = client->ip();
            const auto clientID = client->id();
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, sf_packet, clientIP);
        }
    }
}
void Server::internal_send_to_all_tcp(Engine::Networking::ServerClient* exclusion, void* data, size_t size) {
    for (auto& client : clients()) {
        if (client != exclusion) {
            auto status = client->send(data, size);
        }
    }
}
void Server::internal_send_to_all_udp(Engine::Networking::ServerClient* exclusion, void* data, size_t size) {
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
sf::Socket::Status Server::send_to_client(Engine::Networking::ServerClient& client, Engine::Networking::Packet& packet) {
    return client.send(packet);
}
sf::Socket::Status Server::send_to_client(Engine::Networking::ServerClient& client, sf::Packet& sf_packet) {
    return client.send(sf_packet);
}
sf::Socket::Status Server::send_to_client(Engine::Networking::ServerClient& client, void* data, size_t size) {
    return client.send(data, size);
}
sf::Socket::Status Server::send_to_client(Engine::Networking::ServerClient& client, void* data, size_t size, size_t& sent) {
    return client.send(data, size, sent);
}
void Server::send_to_all_but_client(Engine::Networking::ServerClient& exclusion, Engine::Networking::Packet& packet) {
    internal_send_to_all_tcp(&exclusion, packet);
}
void Server::send_to_all_but_client(Engine::Networking::ServerClient& exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_tcp(&exclusion, sf_packet);
}
void Server::send_to_all_but_client(Engine::Networking::ServerClient& exclusion, void* data, size_t size) {
    internal_send_to_all_tcp(&exclusion, data, size);
}
void Server::send_to_all(Engine::Networking::Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}
void Server::send_to_all(sf::Packet& packet) {
    internal_send_to_all_tcp(nullptr, packet);
}
void Server::send_to_all(void* data, size_t size) {
    internal_send_to_all_tcp(nullptr, data, size);
}
void Server::send_to_all_but_client_udp(Engine::Networking::ServerClient& exclusion, Engine::Networking::Packet& packet) {
    internal_send_to_all_udp(&exclusion, packet);
}
void Server::send_to_all_but_client_udp(Engine::Networking::ServerClient& exclusion, sf::Packet& sf_packet) {
    internal_send_to_all_udp(&exclusion, sf_packet);
}
void Server::send_to_all_but_client_udp(Engine::Networking::ServerClient& exclusion, void* data, size_t size) {
    internal_send_to_all_udp(&exclusion, data, size);
}
void Server::send_to_all_udp(Engine::Networking::Packet& packet) {
    internal_send_to_all_udp(nullptr, packet);
}
void Server::send_to_all_udp(sf::Packet& sf_packet) {
    internal_send_to_all_udp(nullptr, sf_packet);
}
void Server::send_to_all_udp(void* data, size_t size) {
    internal_send_to_all_udp(nullptr, data, size);
}
sf::Socket::Status Server::receive_udp(sf::Packet& sf_packet) {
    return m_UdpSocket->receive(sf_packet);
}
sf::Socket::Status Server::receive_udp(void* data, size_t size, size_t& received) {
    return m_UdpSocket->receive(data, size, received);
}


void Server::on_recieve_udp(sf::Packet& sf_packet) {

}
void Server::internal_add_client(string& hash, Engine::Networking::ServerClient& client) {
    auto* next_thread = internal_get_next_available_thread();
    if (next_thread) {
        next_thread->add_client(hash, &client);
        cout << "Server: TCP listener accepted new client: " << client.ip() << " on port: " << client.port() << "\n";
    }
}
void Server::remove_client(Engine::Networking::ServerClient& client) {
    for (auto& clientThread : m_Threads) {
        auto& clients = clientThread.clients();
        for (auto& client_itr : clients) {
            if (client_itr.second == &client) {
                //std::cout << "Client: " << client_itr.second->ip() << " - has been completely removed from the server" << std::endl;
                {
                    std::lock_guard lock(m_Mutex);
                    clients.erase(client_itr.first);
                }
                return;
            }
        }
    }
}

ServerClient* Server::add_new_client(string& hash, string& clientIP, unsigned short clientPort, Engine::Networking::SocketTCP* tcp) {
    ServerClient* client = NEW ServerClient(hash, *this, tcp);
    return client;
}
void Server::internal_update_tcp_listener_loop() {
    if (m_TCPListener) {
        auto* tcp_socket     = NEW Engine::Networking::SocketTCP();
        auto status          = m_TCPListener->accept(*tcp_socket);
        if (status == sf::Socket::Status::Done) {
            auto client_ip   = tcp_socket->ip();
            auto client_port = tcp_socket->remotePort();
            auto client_hash = m_Client_Hash_Function(client_ip, client_port);
            auto* new_client = add_new_client(client_hash, client_ip, client_port, tcp_socket);
            if (new_client) {
                internal_add_client(client_hash, *new_client);
            }
        }else{
            delete tcp_socket;
        }
    }
}
void Server::onEvent(const Event& e) {

}
void Server::internal_update_loop(const float dt) {
    const auto server_active = m_Active.load(std::memory_order_relaxed);
    if (server_active == 1) {
        internal_update_tcp_listener_loop();

        //server udp socket
        sf::Packet sf_packet_udp;
        auto status_udp = receive_udp(sf_packet_udp);
        if (status_udp == sf::Socket::Done) {
            on_recieve_udp(sf_packet_udp);
        }

        //TODO: it works but it's kind of hacky ///////////////////////////////////////////////////
        auto lambda_update_client_thread = [dt, this, server_active](Engine::Networking::ServerThread& client_thread) {
            for (auto& client_itr : client_thread.clients()) {
                if (server_active == 0 || client_thread.num_clients() == 0) { //can be inactive if no clients are using the thread
                    return true;
                }
                if (client_itr.second) {
                    auto& client = *client_itr.second;
                    client.internal_update_loop(dt);
                    client.update(dt);
                }
            }
            return false;
        };
        for (auto& clientThread : m_Threads) {
            Engine::priv::threading::addJobRef(lambda_update_client_thread, clientThread);
        }
        //////////////////////////////////////////////////////////////////////////////////////////////
    }
}
void Server::update(const float dt) {
    internal_update_loop(dt);
    m_Update_Function(dt);
}
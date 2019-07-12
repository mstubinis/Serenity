#include "Server.h"
#include "Packet.h"

#include <core/engine/Engine_Utils.h>

#include <core/engine/Engine_Math.h>

#include <iostream>

using namespace std;

using namespace Engine;
using namespace Engine::Networking;


Server::Server(const unsigned int& port) {
    m_port         = port;
    m_active       = false;
    m_listener     = new ListenerTCP(m_port, "127.0.0.1");
    //m_thread       = nullptr;
}
Server::~Server() {
    shutdown();
}

const bool Server::startup() {
    if (!m_active) {
        const sf::Socket::Status status = m_listener->listen();
        if (status == sf::Socket::Status::Done) {
            m_active = true;
            std::cout << "Server has started on ip: " << m_listener->ip() << ", and port: " << m_listener->localPort() << std::endl;

            m_listener->setBlocking(false);

            //m_thread = new std::thread(epriv::ServerInternalPublicInterface::update,this);
            //m_thread->detach();
            return true;
        }
    }
    return false;
}
void Server::shutdown() {
    if (m_active) {

        //alert all the clients that the server is shutting down
        for (auto& client : m_clients) {
            Packet p;
            p.PacketType = PacketType::ServerShutdown;
            client.second->send(p);
        }
        //then close the clients
        for (auto& client : m_clients) {
            client.second->disconnect();
            SAFE_DELETE(client.second);
        }
        //SAFE_DELETE(m_thread);
        m_clients.clear();
        //m_listener->close();
        SAFE_DELETE(m_listener);
        m_active = false;

        std::cout << "Server has been shutdown" << std::endl;
    }
}
const bool& Server::isActive() const {
    return m_active;
}

void Server::onReceive() {
    //clients are NON BLOCKING
    for (auto& itr : m_clients) {
        auto& client = *itr.second;
        sf::Packet sf_packet;
        const auto& status = client.receive(sf_packet);
        if (status != sf::Socket::Done){
            // error...
        }
        Packet p;



        if (sf_packet >> p.PacketType >> p.x >> p.y >> p.z >> p.x1 >> p.y1 >> p.z1 >> p.x2 >> p.y2 >> p.z2 >> p.x3 >> p.y3 >> p.z3) {
            // Data extracted successfully...

            float x, y, z, x1, y1, z1, x2, y2, z2, x3, y3, z3;
            Engine::Math::Float32From16(&x, p.x);
            Engine::Math::Float32From16(&y, p.y);
            Engine::Math::Float32From16(&z, p.z);

            Engine::Math::Float32From16(&x1, p.x1);
            Engine::Math::Float32From16(&y1, p.y1);
            Engine::Math::Float32From16(&z1, p.z1);

            Engine::Math::Float32From16(&x2, p.x2);
            Engine::Math::Float32From16(&y2, p.y2);
            Engine::Math::Float32From16(&z2, p.z2);

            Engine::Math::Float32From16(&x3, p.x3);
            Engine::Math::Float32From16(&y3, p.y3);
            Engine::Math::Float32From16(&z3, p.z3);

            std::cout << "Server: "
                << static_cast<int>(p.PacketType)
                << ", "
                << x
                << ", "
                << y
                << ", "
                << z
                << ", "
                << x1
                << ", "
                << y1
                << ", "
                << z1
                << ", "
                << x2
                << ", "
                << y2
                << ", "
                << z2
                << ", "
                << x3
                << ", "
                << y3
                << ", "
                << z3
            << std::endl;

            send_to_client(client, p);
        }
    }
}


const sf::Socket::Status Server::send_to_client(Client& c, Packet& packet) {
    return c.send(packet);
}
const sf::Socket::Status Server::send_to_client(Client& c, sf::Packet& packet) {
    return c.send(packet);
}
const sf::Socket::Status Server::send_to_client(Client& c, const void* data, size_t size) {
    return c.send(data, size);
}
const sf::Socket::Status Server::send_to_client(Client& c, const void* data, size_t size, size_t& sent) {
    return c.send(data, size, sent);
}


const void Server::send_to_all_but_client(Client& c, Packet& packet) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            cl.second->send(packet);
        }
    }
}
const void Server::send_to_all_but_client(Client& c, sf::Packet& packet) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            cl.second->send(packet);
        }
    }
}
const void Server::send_to_all_but_client(Client& c, const void* data, size_t size) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            cl.second->send(data, size);
        }
    }
}
const void Server::send_to_all_but_client(Client& c, const void* data, size_t size, size_t& sent) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            cl.second->send(data, size, sent);
        }
    }
}


const void Server::send_to_all(Packet& packet) {
    for (auto& cl : m_clients) {
        cl.second->send(packet);
    }
}
const void Server::send_to_all(sf::Packet& packet) {
    for (auto& cl : m_clients) {
        cl.second->send(packet);
    }
}
const void Server::send_to_all(const void* data, size_t size) {
    for (auto& cl : m_clients) {
        cl.second->send(data, size);
    }
}
const void Server::send_to_all(const void* data, size_t size, size_t& sent) {
    for (auto& cl : m_clients) {
        cl.second->send(data, size, sent);
    }
}




void epriv::ServerInternalPublicInterface::update(Server* _server) {
    if (!_server) return;
    auto& server = *_server;
    //while (server.isActive()) {
        sf::TcpSocket* client = new sf::TcpSocket();
        const auto& status = server.m_listener->accept(*client);
        if (status == sf::Socket::Status::Done) {
            const auto& client_ip      = client->getRemoteAddress().toString();
            const auto& client_port    = client->getRemotePort();
            const auto& client_address = client_ip + " " + to_string(client_port);
            if (!server.m_clients.count(client_address)) {
                Client* serverClient = new Client(client);
                client->setBlocking(false);
                server.m_clients.emplace(client_address, serverClient);
                std::cout << "New client: " << client_address << std::endl;
            }
        }else{
            SAFE_DELETE(client);
        }
        server.onReceive();
    //}
}

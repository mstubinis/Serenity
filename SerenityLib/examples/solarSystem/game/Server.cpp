#include "Server.h"
#include "Packet.h"

#include "SolarSystem.h"
#include <core/engine/resources/Engine_Resources.h>
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
            p.PacketType = PacketType::Server_Shutdown;
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
        if (status == sf::Socket::Done){
            Packet* pp = Packet::getPacket(sf_packet);
            auto& p = *pp;
            if (pp && p.validate(sf_packet)) {
                // Data extracted successfully...
                std::cout << "Server: ";
                p.print();

                if (p.PacketType == PacketType::Client_To_Server_Ship_Physics_Update) {
                    if (p.data != "") {
                        SolarSystem& scene = *static_cast<SolarSystem*>(Resources::getCurrentScene());
                        PacketPhysicsUpdate physics_update(*scene.getShips().at(p.data));


                        physics_update.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;
                        client.send(physics_update);
                    }
                }
            }
            SAFE_DELETE(pp);
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
        sf::TcpSocket* sf_client = new sf::TcpSocket();
        const auto& status = server.m_listener->accept(*sf_client);
        if (status == sf::Socket::Status::Done) {
            const auto& client_ip      = sf_client->getRemoteAddress().toString();
            const auto& client_port    = sf_client->getRemotePort();
            const auto& client_address = client_ip + " " + to_string(client_port);
            if (!server.m_clients.count(client_address)) {
                sf_client->setBlocking(false);
                Client* client = new Client(sf_client);
                server.m_clients.emplace(client_address, client);
                std::cout << "New client: " << client_address << std::endl;

                Packet p;
                p.PacketType = PacketType::Server_To_Client_Accept_Connection;
                p.data = "Client_Accepted";

                client->send(p);
            }
        }else{
            SAFE_DELETE(sf_client);
        }
        server.onReceive();
    //}
}

#include "Server.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "Helper.h"
#include "ResourceManifest.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"

#include "SolarSystem.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Utils.h>
#include <core/engine/math/Engine_Math.h>

#include <core/engine/Engine.h>
#include <core/engine/Engine_Debugging.h>

#include <iostream>

using namespace std;

using namespace Engine;
using namespace Engine::Networking;

//TODO: wrap thead functions in while loops

Server::Server(Core& core, const unsigned int& port, const bool blocking, const string& ipRestriction) :m_Core(core) {
    m_port                             = port;
    m_listener                         = new ListenerTCP(port, ipRestriction);
    m_blocking                         = blocking;
    m_thread_for_listener              = nullptr;
    m_thread_for_disconnecting_clients = nullptr;
}
Server::~Server() {
    shutdown(true);
}

const bool Server::startup(const string& mapname) {
    auto& listener = *m_listener;
    m_MapName = mapname;
    const sf::Socket::Status status = listener.listen();
    if (status == sf::Socket::Status::Done) {
        cout << "Server has started on port: " << listener.localPort() << endl;
        listener.setBlocking(m_blocking);
        if (m_blocking) {
            SAFE_DELETE_THREAD(m_thread_for_listener);
            m_thread_for_listener = new std::thread(Server::updateAcceptNewClients,this);
            m_thread_for_listener->detach();
            SAFE_DELETE_THREAD(m_thread_for_disconnecting_clients);
            m_thread_for_disconnecting_clients = new std::thread(Server::updateRemoveDisconnectedClients, this);
            m_thread_for_disconnecting_clients->detach();
        }
        return true;
    }
    return false;
}
void Server::shutdown(bool destructor) {
    //alert all the clients that the server is shutting down
    Packet p;
    p.PacketType = PacketType::Server_Shutdown;
    send_to_all(p);
    m_Core.m_HUD->m_ServerLobbyChatWindow->clear();
    m_Core.m_HUD->m_ServerLobbyConnectedPlayersWindow->clear();

    if (destructor) {
        SAFE_DELETE_MAP(m_clients);
        SAFE_DELETE_MAP(m_threads_for_clients);
        SAFE_DELETE_THREAD(m_thread_for_listener);
        SAFE_DELETE_THREAD(m_thread_for_disconnecting_clients);
        SAFE_DELETE(m_listener);
    }else{
        m_listener->close();
    }
}
void Server::update(Server* thisServer) {
    if (thisServer->m_blocking)
        return; //use threads instead
    updateAcceptNewClients(thisServer); //if blocking, should be its own thread
    for (auto& itr : thisServer->m_clients) {
        auto& client = *itr.second;
        updateClient(thisServer ,&client);//if blocking, should be its own thread
    }
    updateRemoveDisconnectedClients(thisServer); //if blocking, should be its own thread
}
void Server::updateRemoveDisconnectedClients(Server* thisServer) {
    thisServer->m_mutex.lock();
    auto& server = *thisServer;
    while (server.m_ClientsToBeDisconnected.size() > 0) {
        server.m_clients.erase(server.m_ClientsToBeDisconnected.front());
        server.m_ClientsToBeDisconnected.pop();
    }
    thisServer->m_mutex.unlock();
}

void Server::updateAcceptNewClients(Server* thisServer) {
    thisServer->m_mutex.lock();
    auto& server = *thisServer;
    sf::TcpSocket* sf_client = new sf::TcpSocket();
    auto& sfClient           = *sf_client;
    const auto& status = server.m_listener->accept(sfClient);
    if (status == sf::Socket::Status::Done) {
        const auto& client_ip      = sfClient.getRemoteAddress().toString();
        const auto& client_port    = sfClient.getRemotePort();
        const auto& client_address = client_ip + " " + to_string(client_port);
        //add client to the server
        if (!server.m_clients.count(client_address)) {
            sfClient.setBlocking(false);
            Client* client = new Client(server.m_Core, sf_client);
            server.m_clients.emplace(client_address, client);
            if (server.m_blocking) {
                std::thread* client_thread = new std::thread(Server::updateClient, thisServer, client);
                client_thread->detach();
                thisServer->m_threads_for_clients.emplace(client_address, client_thread);
            }
            cout << "Server: New client in dictionary: " << client_address << endl;
        }
    }else{ SAFE_DELETE(sf_client); }
    thisServer->m_mutex.unlock();
}
void Server::updateClient(Server* thisServer, Client* _client) {
    auto& server = *thisServer;
    server.m_mutex.lock();
    sf::Packet sf_packet;
    auto& client = *_client;
    const auto& status = client.receive(sf_packet);
    if (status == sf::Socket::Done) {
        Packet* pp = Packet::getPacket(sf_packet);
        auto& pIn = *pp;
        if (pp && pIn.validate(sf_packet)) {
            // Data extracted successfully...
            switch (pIn.PacketType) {
                case PacketType::Client_To_Server_Ship_Physics_Update: {
                    auto& map = *static_cast<SolarSystem*>(Resources::getCurrentScene());

                    //a client has sent the server it's physics information, lets forward it to the rest of the clients
                    PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(pp);
                    PacketPhysicsUpdate pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;
                    server.send_to_all_but_client(client, pOut);
                    break;
                }
                case PacketType::Client_To_Server_Request_Map_Entry: {

                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut;
                    PacketMessage pOut1;

                    auto x = Helper::GetRandomFloatFromTo(150, 180);
                    auto y = Helper::GetRandomFloatFromTo(150, 180);
                    auto z = Helper::GetRandomFloatFromTo(150, 180);


                    pOut.PacketType = PacketType::Server_To_Client_Approve_Map_Entry;
                    pOut.name = pI.name;
                    pOut.data = pI.data;
                    pOut.r = x;
                    pOut.g = y;
                    pOut.b = z;
                    server.send_to_client(client, pOut);

                    
                    pOut1.PacketType = PacketType::Server_To_Client_New_Client_Entered_Map;
                    pOut1.name = pI.name;
                    pOut1.data = pI.data;
                    pOut1.r = x;
                    pOut1.g = y;
                    pOut1.b = z;
                    server.send_to_all_but_client(client, pOut1);
                    
                    break;
                }
                case PacketType::Client_To_Server_Chat_Message: {
                    PacketMessage pOut;
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    pOut.PacketType = PacketType::Server_To_Client_Chat_Message;
                    pOut.data = pI.data;
                    pOut.name = pI.name;
                    pOut.r = pI.r;
                    pOut.g = pI.g;
                    pOut.b = pI.b;
                    server.send_to_all(pOut);
                    break;
                }
                case PacketType::Client_To_Server_Request_Connection: {
                    const bool valid = server.isValidName(pIn.data);
                    Packet pOut;
                    if (valid) {
                        client.m_username = pIn.data;
                        client.m_Validated = true;
                        pOut.PacketType = PacketType::Server_To_Client_Accept_Connection;
                        pOut.data = "";
                        for (auto& c : server.m_clients) {
                            if(!c.second->m_username.empty() && c.second->m_username != client.m_username)
                                pOut.data += c.second->m_username + ",";
                        }
                        if(!pOut.data.empty())
                            pOut.data.pop_back();
                        cout << "Server: Approving: " + pIn.data + "'s connection" << endl;

                        PacketMessage pOut1;
                        pOut1.name = client.m_username;
                        pOut1.data = "";
                        pOut1.PacketType = PacketType::Server_To_Client_Client_Joined_Server;


                        PacketMessage pOut2;
                        pOut2.name = server.m_MapName;
                        SolarSystem* map = static_cast<SolarSystem*>(Resources::getScene(server.m_MapName));
                        if (!map) {
                            map = new SolarSystem(server.m_MapName, ResourceManifest::BasePath + "data/Systems/" + server.m_MapName + ".txt");
                        }
                        pOut2.data = map->allowedShipsSingleString();
                        pOut2.PacketType = PacketType::Server_To_Client_Map_Data;

                        server.send_to_client(client, pOut);
                        server.send_to_all(pOut1);
                        server.send_to_client(client, pOut2);
                    }else{
                        pOut.PacketType = PacketType::Server_To_Client_Reject_Connection;
                        cout << "Server: Rejecting: " + pIn.data + "'s connection" << endl;
                        server.send_to_client(client, pOut);
                    }
                    break;
                }case PacketType::Client_To_Server_Request_Disconnection: {
                    cout << "Server: Removing " + client.m_username + " from the server" << endl;
                    const auto& client_ip      = client.m_TcpSocket->ip();
                    const auto& client_port    = client.m_TcpSocket->remotePort();
                    const auto& client_address = client_ip + " " + to_string(client_port);

                    PacketMessage pOut1;
                    pOut1.name = client.m_username;
                    pOut1.data = "";
                    pOut1.PacketType = PacketType::Server_To_Client_Client_Left_Server;
                    server.send_to_all(pOut1);

                    client.disconnect();
                    server.m_ClientsToBeDisconnected.push(client_address);
                    break;
                }default:{
                    break;
                }
            }
        }
        SAFE_DELETE(pp);
    }
    server.m_mutex.unlock();
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
void Server::send_to_all_but_client(Client& c, Packet& packet) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(packet);
        }
    }
}
void Server::send_to_all_but_client(Client& c, sf::Packet& packet) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(packet);
        }
    }
}
void Server::send_to_all_but_client(Client& c, const void* data, size_t size) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(data, size);
        }
    }
}
void Server::send_to_all_but_client(Client& c, const void* data, size_t size, size_t& sent) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(data, size, sent);
        }
    }
}
void Server::send_to_all(Packet& packet) {
    for (auto& cl : m_clients) {
        auto status = cl.second->send(packet);
    }
}
void Server::send_to_all(sf::Packet& packet) {
    for (auto& cl : m_clients) {
        auto status = cl.second->send(packet);
    }
}
void Server::send_to_all(const void* data, size_t size) {
    for (auto& cl : m_clients) {
        auto status = cl.second->send(data, size);
    }
}
void Server::send_to_all(const void* data, size_t size, size_t& sent) {
    for (auto& cl : m_clients) {
        auto status = cl.second->send(data, size, sent);
    }
}
const bool Server::isValidName(const string& name) const {
    if (name.empty())
        return false;
    for (auto& client : m_clients) {
        if (client.second->m_username == name) {
            return false;
        }
    }
    return true;
}
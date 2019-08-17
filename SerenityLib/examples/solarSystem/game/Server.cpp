#include "Server.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "Helper.h"
#include "ResourceManifest.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"

#include "map/Map.h"
#include "map/Anchor.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/math/Engine_Math.h>

#include <core/engine/Engine.h>
#include <core/engine/utils/Engine_Debugging.h>

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace std;

using namespace Engine;
using namespace Engine::Networking;

const double TIMER_DEEP_SPACE_ANCHOR_SPAM = 1.0;

//TODO: wrap thead functions in while loops

#pragma region ServerClient

ServerClient::ServerClient(Core& core, sf::TcpSocket* sfTCPSocket) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(sfTCPSocket);
    internalInit();
}
ServerClient::ServerClient(Core& core, const ushort& port, const string& ipAddress) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    internalInit();
}
ServerClient::~ServerClient() {
    SAFE_DELETE(m_TcpSocket);
}
void ServerClient::internalInit() {
    m_RecoveryTime = 0.0f;
    m_Timeout      = 0.0f;
    m_username     = "";
    m_Validated    = false;
}
const bool ServerClient::disconnected() const {
    return (m_Timeout >= SERVER_CLIENT_TIMEOUT) ? true : false;
}
void ServerClient::disconnect() {
    m_TcpSocket->disconnect();
}
const sf::Socket::Status ServerClient::send(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_TcpSocket->send(sf_packet);
    return status;
}
const sf::Socket::Status ServerClient::send(sf::Packet& packet) {
    const auto status = m_TcpSocket->send(packet);
    return status;
}
const sf::Socket::Status ServerClient::send(const void* data, size_t size) {
    const auto status = m_TcpSocket->send(data, size);
    return status;
}
const sf::Socket::Status ServerClient::send(const void* data, size_t size, size_t& sent) {
    const auto status = m_TcpSocket->send(data, size, sent);
    return status;
}
const sf::Socket::Status ServerClient::receive(sf::Packet& packet) {
    const auto status = m_TcpSocket->receive(packet);
    return status;
}
const sf::Socket::Status ServerClient::receive(void* data, size_t size, size_t& received) {
    const auto status = m_TcpSocket->receive(data, size, received);
    return status;
}
const string& ServerClient::username() const {
    return m_username;
}

#pragma endregion


#pragma region Server

Server::Server(Core& core, const unsigned int& port, const bool blocking, const string& ipRestriction) :m_Core(core) {
    m_port                             = port;
    m_listener                         = new ListenerTCP(port, ipRestriction);
    m_blocking                         = blocking;
    m_thread_for_listener              = nullptr;
    m_thread_for_disconnecting_clients = nullptr;
    m_DeepspaceAnchorTimer             = 0.0f;
}
Server::~Server() {
    shutdown(true);
}

const bool Server::startup(const string& mapname) {
    auto& listener = *m_listener;
    m_MapName = mapname;
    const sf::Socket::Status status = listener.listen();
    if (status == sf::Socket::Status::Done) {
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
    Sleep(500); //messy
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
    const auto& dt = Resources::dt();
    thisServer->m_DeepspaceAnchorTimer += dt;
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
            ServerClient* client = new ServerClient(server.m_Core, sf_client);
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
void Server::updateClient(Server* thisServer, ServerClient* _client) {
    auto& server = *thisServer;
    server.m_mutex.lock();
    sf::Packet sf_packet;
    auto& client = *_client;

    auto fdt = static_cast<float>(Resources::dt());
    if (client.disconnected()) {
        client.m_RecoveryTime += fdt;
        if (client.m_RecoveryTime > SERVER_CLIENT_RECOVERY_TIME) {
            std::cout << "Client: " << client.m_username << " has fully timed out, removing him completely" << std::endl;
            //notify the other players of his removal
            PacketMessage pOut;
            pOut.PacketType = PacketType::Server_To_Client_Client_Left_Map;
            pOut.name = client.m_username;
            server.send_to_all_but_client(client, pOut);
            for (auto& c : server.m_clients) {
                if (c.second->m_username == client.m_username) {
                    SAFE_DELETE(c.second);
                    server.m_clients.erase(c.first);
                    break;
                }
            }
            return;
        }
    }else{
        client.m_Timeout += fdt;
        if (client.m_Timeout > SERVER_CLIENT_TIMEOUT) {
            std::cout << "Client: " << client.m_username << " has timed out, disconnecting..." << std::endl;
            client.disconnect();
            return;
        }
        const auto status = client.receive(sf_packet);
        if (status == sf::Socket::Done) {
            Packet* pp = Packet::getPacket(sf_packet);
            auto& pIn = *pp;
            if (pp && pIn.validate(sf_packet)) {
                client.m_Timeout = 0.0f;
                // Data extracted successfully...
                switch (pIn.PacketType) {
                    case PacketType::Client_To_Server_Periodic_Ping: {
                        //thanks for staying with us
                        client.m_Timeout = 0.0f;
                        break;
                    }case PacketType::Client_To_Server_Client_Fired_Beams: {
                        //just forward it
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Client_Fired_Beams;
                        server.send_to_all_but_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Client_Fired_Cannons: {
                        //just forward it
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Client_Fired_Cannons;
                        server.send_to_all_but_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Client_Fired_Torpedos: {
                        //just forward it
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Client_Fired_Torpedos;
                        server.send_to_all_but_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Client_Changed_Target: {
                        //just forward it
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Client_Changed_Target;
                        server.send_to_all_but_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Successfully_Entered_Map: {
                        //client told me he entered, lets give him his data to the other clients, and give him info about the current deep space anchors
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut1(pI);

                        pOut1.PacketType = PacketType::Server_To_Client_New_Client_Entered_Map;
                        server.send_to_all_but_client(client, pOut1);

                        auto list = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass, map

                        auto& map = *static_cast<Map*>(Resources::getScene(list[1]));
                        auto spawnPosition = map.getSpawnAnchor()->getPosition();
                        for (auto& anchor : map.getRootAnchor()->getChildren()) {
                            if (boost::contains(anchor.first, "Deepspace Anchor")) {
                                auto anchorPosition = anchor.second->getPosition();
                                PacketMessage pOut2;
                                pOut2.PacketType = PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial;
                                pOut2.r = anchorPosition.x - spawnPosition.x;
                                pOut2.g = anchorPosition.y - spawnPosition.y;
                                pOut2.b = anchorPosition.z - spawnPosition.z;
                                pOut2.data = anchor.first;
                                server.send_to_client(client, pOut2);
                            }
                        }
                        break;
                    }case PacketType::Client_To_Server_Request_Anchor_Creation: {
                        if (server.m_DeepspaceAnchorTimer > TIMER_DEEP_SPACE_ANCHOR_SPAM) {
                            //just forward it
                            PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                            PacketMessage pOut(pI);
                            pOut.PacketType = PacketType::Server_To_Client_Anchor_Creation;
                            server.send_to_all(pOut);
                            server.m_DeepspaceAnchorTimer = 0.0;
                        }
                        break;
                    }case PacketType::Client_To_Server_Ship_Health_Update:{
                        //just forward it
                        PacketHealthUpdate& pI = *static_cast<PacketHealthUpdate*>(pp);
                        PacketHealthUpdate pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Ship_Health_Update;
                        server.send_to_all_but_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Ship_Cloak_Update: {
                        //a client has sent us it's cloaking information, forward it
                        PacketCloakUpdate& pI = *static_cast<PacketCloakUpdate*>(pp);
                        PacketCloakUpdate pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Ship_Cloak_Update;

                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        if (info.size() >= 3) {
                            for (auto& connectedClients : server.m_clients) {
                                if (connectedClients.second->m_username == info[2]) {
                                    server.send_to_client(*connectedClients.second, pOut);
                                    break;
                                }
                            }
                        }else{
                            server.send_to_all_but_client(client, pOut);
                        }
                        break;
                    }case PacketType::Client_To_Server_Ship_Physics_Update: {
                        //a client has sent the server it's physics information, lets forward it to the rest of the clients
                        PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(pp);
                        PacketPhysicsUpdate pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;
                        server.send_to_all_but_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Request_Map_Entry: {
                        //ok client, you told me you want in, lets approve you -> just bouncing
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Approve_Map_Entry;
                        server.send_to_client(client, pOut);
                        break;
                    }case PacketType::Client_To_Server_Chat_Message: {
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
                    }case PacketType::Client_To_Server_Request_Connection: {
                        const bool valid = server.isValidName(pIn.data);
                        Packet pOut;
                        if (valid) {
                            client.m_username = pIn.data;
                            client.m_Validated = true;
                            pOut.PacketType = PacketType::Server_To_Client_Accept_Connection;
                            pOut.data = "";
                            for (auto& c : server.m_clients) {
                                if (!c.second->m_username.empty() && c.second->m_username != client.m_username) {
                                    pOut.data += c.second->m_username + ",";
                                }
                            }
                            if (!pOut.data.empty())
                                pOut.data.pop_back();
                            cout << "Server: Approving: " + pIn.data + "'s connection" << endl;

                            PacketMessage pOut1;
                            pOut1.name = client.m_username;
                            pOut1.data = "";
                            pOut1.PacketType = PacketType::Server_To_Client_Client_Joined_Server;


                            PacketMessage pOut2;
                            pOut2.name = server.m_MapName;
                            Map* map = static_cast<Map*>(Resources::getScene(server.m_MapName));
                            if (!map) {
                                map = new Map(server.m_MapName, ResourceManifest::BasePath + "data/Systems/" + server.m_MapName + ".txt");
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
                        const auto& client_ip = client.m_TcpSocket->ip();
                        const auto& client_port = client.m_TcpSocket->remotePort();
                        const auto& client_address = client_ip + " " + to_string(client_port);

                        PacketMessage pOut1;
                        pOut1.name = client.m_username;
                        pOut1.data = "";
                        pOut1.PacketType = PacketType::Server_To_Client_Client_Left_Server;
                        server.send_to_all(pOut1);

                        client.disconnect();
                        server.m_ClientsToBeDisconnected.push(client_address);
                        break;
                    }default: {
                        break;
                    }
                }
            }
            SAFE_DELETE(pp);
        }
    }
    server.m_mutex.unlock();
}
const sf::Socket::Status Server::send_to_client(ServerClient& c, Packet& packet) {
    return c.send(packet);
}
const sf::Socket::Status Server::send_to_client(ServerClient& c, sf::Packet& packet) {
    return c.send(packet);
}
const sf::Socket::Status Server::send_to_client(ServerClient& c, const void* data, size_t size) {
    return c.send(data, size);
}
const sf::Socket::Status Server::send_to_client(ServerClient& c, const void* data, size_t size, size_t& sent) {
    return c.send(data, size, sent);
}
void Server::send_to_all_but_client(ServerClient& c, Packet& packet) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(packet);
        }
    }
}
void Server::send_to_all_but_client(ServerClient& c, sf::Packet& packet) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(packet);
        }
    }
}
void Server::send_to_all_but_client(ServerClient& c, const void* data, size_t size) {
    for (auto& cl : m_clients) {
        if (cl.second != &c) {
            auto status = cl.second->send(data, size);
        }
    }
}
void Server::send_to_all_but_client(ServerClient& c, const void* data, size_t size, size_t& sent) {
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

#pragma endregion
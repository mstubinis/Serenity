#include "Server.h"
#include "ServerMapSpecificData.h"
#include "../Packet.h"
#include "../client/Client.h"
#include "../../Core.h"
#include "../../Menu.h"
#include "../../Helper.h"
#include "../../ResourceManifest.h"
#include "../../gui/specifics/ServerLobbyChatWindow.h"
#include "../../gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "../../modes/GameplayMode.h"

#include "../../map/Map.h"
#include "../../map/Anchor.h"
#include "../../teams/Team.h"
#include "../../modes/GameplayMode.h"
#include "../../ships/Ships.h"

#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"
#include "../../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>

#include <core/engine/Engine.h>
#include <core/engine/utils/Engine_Debugging.h>

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;


#pragma region ServerClient

ServerClient::ServerClient(const string& hash, Server& server, Core& core, sf::TcpSocket* sfTCPSocket) : m_Core(core), m_Server(server) {
    m_TcpSocket = new Networking::SocketTCP(sfTCPSocket);
    internalInit(hash, server.numClients());
}
ServerClient::ServerClient(const string& hash, Server& server, Core& core, const unsigned short& port, const string& ipAddress) : m_Core(core), m_Server(server) {
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    internalInit(hash, server.numClients());
}
ServerClient::~ServerClient() {
    SAFE_DELETE(m_TcpSocket);
}
void ServerClient::internalInit(const string& hash, const unsigned int& numClients) {
    m_TcpSocket->setBlocking(false);
    m_RecoveryTime = 0.0;
    m_Hash         = hash;
    m_Timeout      = 0.0;
    m_Username     = "";
    m_MapKey       = "";
    m_Validated    = false;
    m_IP = m_TcpSocket->ip();
    m_ID = numClients + 1;
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
    return m_Username;
}
const string& ServerClient::getMapKey() const {
    return m_MapKey;
}

#pragma endregion

#pragma region ServerClientThread
ServerClientThread::ServerClientThread(){
    m_Thread = nullptr;
    m_Active.store(0, std::memory_order_relaxed);
    m_Shutdowned = false;

    //TODO: works, but interferes with the thread pool due to over-thread usage and causes major slowdowns because of it, using thread pool directly for now
    /*
    auto updateClientThread = [&](ServerClientThread& clientThread) {
        while (!clientThread.m_Shutdowned) {
            for (auto& clientItr : clientThread.m_Clients) {
                auto& client = *clientItr.second;
                const auto server_active = client.m_Server.m_Active.load(std::memory_order_relaxed);
                const auto thread_active = clientThread.m_Active.load(std::memory_order_relaxed); //can be inactive if no clients are using the thread
                if (server_active == 0 || thread_active == 0)
                    return true;
                Server::updateClient(client);
            }
        }
        return false;
    };
    m_Thread = new std::future<bool>(std::move(std::async(std::launch::async, updateClientThread, std::ref(*this))));
    */    
}
ServerClientThread::~ServerClientThread() {
    m_Shutdowned = true;
    m_Active.store(0, std::memory_order_relaxed);
    SAFE_DELETE_FUTURE(m_Thread);
    SAFE_DELETE_MAP(m_Clients);

}
#pragma endregion

#pragma region Server

Server::Server(Core& core, const unsigned int& port, const string& ipRestriction) : m_Core(core), m_MapSpecificData(*this){
    m_GameplayMode                     = nullptr;
    m_OwnerClient                      = nullptr;
    m_port                             = port;
    m_listener                         = new ListenerTCP(port, ipRestriction);
    //m_PingTime                         = 0.0;
    m_UdpSocket                        = new SocketUDP(port, ipRestriction);
    m_UdpSocket->setBlocking(false);
    m_UdpSocket->bind();
    m_Active.store(0, std::memory_order_relaxed);
}
Server::~Server() {
    m_UdpSocket->unbind();
    shutdown(true);
}
void Server::shutdown(const bool destructor) {
    //alert all the clients that the server is shutting down
    Packet p;
    p.PacketType = PacketType::Server_Shutdown;
    send_to_all(p);
    m_Core.m_Menu->m_ServerLobbyChatWindow->clear();
    m_Core.m_Menu->m_ServerLobbyConnectedPlayersWindow->clear();
    m_Active.store(0, std::memory_order_relaxed);
    m_OwnerClient = nullptr;
    Sleep(500); //messy
    if (destructor) {
        SAFE_DELETE_VECTOR(m_Threads);
        SAFE_DELETE(m_listener);
        SAFE_DELETE(m_GameplayMode);
    }else{
        m_listener->close();
    }
}
const bool Server::shutdownMap() {
    const auto success = Resources::deleteScene(*m_MapSpecificData.m_Map);
    if (success) {
        Resources::setCurrentScene("Menu");
    }
    return success;
}
const unsigned int Server::numClients() const {
    unsigned int numClients = 0;
    for (auto& thread : m_Threads) {
        for (auto& client : thread->m_Clients) {
            ++numClients;
        }
    }
    return numClients;
}

const bool Server::startup(const string& mapname) {
    auto& listener = *m_listener;
    const sf::Socket::Status status = listener.listen();

    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        m_Threads.push_back(new ServerClientThread());
    }

    if (status == sf::Socket::Status::Done) {
        listener.setBlocking(false);
        m_Active.store(1, std::memory_order_relaxed);
        return true;
    }
    return false;
}
const bool Server::startupMap(const string& mapname, GameplayMode& mode) {
    m_GameplayMode = &mode;
    m_MapSpecificData.m_Map = static_cast<Map*>(Resources::getScene(mapname));
    if (!m_MapSpecificData.m_Map) {
        Map* map = new Map(mode, *m_Core.m_Client, mapname, ResourceManifest::BasePath + "data/Systems/" + mapname + ".txt");
        m_MapSpecificData.m_Map = static_cast<Map*>(Resources::getScene(mapname));
        m_MapSpecificData.m_Map->m_IsServer = true;
        return true;
    }
    return false;
}

void Server::update(Server* thisServer, const double& dt) {
    auto& server = *thisServer;
    const auto server_active = server.m_Active.load(std::memory_order_relaxed);
    if (server_active == 1) {      
        updateAcceptNewClients(server);

        server.m_MapSpecificData.update(dt);

        server.updateClientsGameLoop(dt);

        //TODO: it works but it's kind of hacky, in the future create a standalone server exe ///////////////////////////////////////////////////
        
        auto updateClientThread = [&](ServerClientThread& clientThread) {
            for (auto& clientItr : clientThread.m_Clients) {
                auto& client = *clientItr.second;
                const auto thread_active = clientThread.m_Active.load(std::memory_order_relaxed);
                if (server_active == 0 || thread_active == 0) //can be inactive if no clients are using the thread
                    return true;
                Server::updateClient(client);
            }
            return false;
        };
        for (auto& thread : server.m_Threads) {
            epriv::threading::addJobRef(updateClientThread, *thread);
        }
        //epriv::threading::waitForAll();
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        updateRemoveDisconnectedClients(server);
    }
}
//Listener thread
void Server::updateAcceptNewClients(Server& server) {
    if (server.m_listener) {
        sf::TcpSocket* sf_client = new sf::TcpSocket();
        auto& sfClient = *sf_client;
        const auto& status = server.m_listener->accept(sfClient);
        if (status == sf::Socket::Status::Done) {
            const auto& client_ip      = sfClient.getRemoteAddress().toString();
            const auto& client_port    = sfClient.getRemotePort();
            const auto& client_address = client_ip + " " + to_string(client_port);

            //get the first thread with the least amount of clients
            ServerClientThread* leastThread = nullptr;
            for (auto& clientThread : server.m_Threads) {
                if (!leastThread || (leastThread && leastThread->m_Clients.size() < clientThread->m_Clients.size())) {
                    leastThread = clientThread;
                }
            }
            if (leastThread) { //probably redundant        
                if (!leastThread->m_Clients.count(client_address)) {
                    sfClient.setBlocking(false);
                    ServerClient* client = new ServerClient(client_address, server, server.m_Core, sf_client);
                    server.m_Mutex.lock();
                    leastThread->m_Clients.emplace(client_address, client);
                    leastThread->m_Active.store(1, std::memory_order_relaxed);
                    if (!server.m_OwnerClient) {
                        server.m_OwnerClient = client;
                    }
                    server.m_Mutex.unlock();
                    std::cout << "Server: New client in dictionary: " << client_address << std::endl;
                }
                
            }
        }else{
            SAFE_DELETE(sf_client);
        }
    }
}
void Server::onReceiveUDP() {
    //udp parsing
    sf::Packet sf_packet_udp;
    const auto status_udp = receive_udp(sf_packet_udp);
    if (status_udp == sf::Socket::Done) {
        Packet* pp = Packet::getPacket(sf_packet_udp);
        auto& pIn = *pp;
        if (pp && pIn.validate(sf_packet_udp)) {
            switch (pIn.PacketType) {
                case PacketType::Client_To_Server_Ship_Physics_Update: {
                    //a client has sent the server it's physics information, lets forward it
                    PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(pp);
                    PacketPhysicsUpdate pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;

                    //get the client who sent it
                    //TODO: this is sort of expensive
                    const auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    ServerClient* client = getClientByUsername(list[1]);
                    if (client) { //this works...
                        client->m_Timeout = 0.0f;
                        send_to_all_but_client_udp(*client, pOut);
                    }
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(pp);
    }
}

//NOT multithreaded
void Server::updateRemoveDisconnectedClients(Server& server) {
    while (server.m_ClientsToBeDisconnected.size() > 0) {
        //std::lock_guard<std::mutex> lock_guard(m_Mutex);
        for (auto& clientThread : server.m_Threads) {
            clientThread->m_Clients.erase(server.m_ClientsToBeDisconnected.front());
        }
        server.m_ClientsToBeDisconnected.pop();
    }
}
void Server::completely_remove_client(ServerClient& client) {
    if (m_OwnerClient) {
        if (m_OwnerClient->m_MapKey == client.m_MapKey) {
            m_OwnerClient = nullptr;
        }
    }
    for (auto& clientThread : m_Threads) {
        for (auto& _client : clientThread->m_Clients) {
            string username_cpy = _client.second->m_Username;
            if (client.m_MapKey == _client.second->m_MapKey) {
                std::cout << "Client: " << username_cpy << " - has been completely removed from the server" << std::endl;
                std::lock_guard<std::mutex> lock_guard(m_Mutex);
                clientThread->m_Clients.erase(_client.first);
                if (clientThread->m_Clients.size() == 0) {
                    clientThread->m_Active.store(0, std::memory_order_relaxed);
                }
                return;
            }
        }
    }
}
void Server::updateClientsGameLoop(const double& dt) {
    const auto active = m_Active.load(std::memory_order_relaxed);
    if (active == 1) {
        onReceiveUDP();

        if (m_Core.m_GameState == GameState::Game) {
            for (auto& clientThread : m_Threads) {
                for (auto& _client : clientThread->m_Clients) {
                    auto& client = *_client.second;
                    if (client.disconnected()) {
                        client.m_RecoveryTime += dt;
                        if (client.m_RecoveryTime > SERVER_CLIENT_RECOVERY_TIME) {
                            //notify the other players of his removal
                            PacketMessage pOut;
                            pOut.PacketType = PacketType::Server_To_Client_Client_Left_Map;
                            pOut.name = client.m_MapKey;
                            send_to_all_but_client(client, pOut);
                            completely_remove_client(client);
                        }
                    }else{
                        client.m_Timeout += dt;
                        if (client.m_Timeout > SERVER_CLIENT_TIMEOUT) {
                            std::cout << "Client: " << client.m_Username << " has timed out, disconnecting..." << std::endl;
                            client.disconnect();
                        }
                    }
                }
            }
        }
    }
}
void Server::updateClient(ServerClient& client) {
    auto& server = client.m_Server;

    sf::Packet sf_packet;
    const auto status     = client.receive(sf_packet);
    if (status == sf::Socket::Done) {
        Packet* pp = Packet::getPacket(sf_packet);
        auto& pIn = *pp;
        if (pp && pIn.validate(sf_packet)) {
            client.m_Timeout = 0.0f;
            // Data extracted successfully...
            switch (pIn.PacketType) {
                case PacketType::Client_To_Server_Ship_Was_Just_Destroyed: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Ship_Was_Just_Destroyed;
                    server.send_to_all_but_client(client, pOut);

                    auto& map = *server.m_MapSpecificData.m_Map;
                    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    if (map.hasShipPlayer(pI.name)) {
                        //this is a player ship, give it permission to respawn, currently NPC's do not respawn
                        server.m_MapSpecificData.m_RespawningShips.processShip(pI.name, list[0], map.getClosestSpawnAnchor());
                    }
                    break;
                }case PacketType::Client_To_Server_Request_Ship_Current_Info: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Request_Ship_Current_Info;


                    auto* c = server.getClientByUsername(list[1]);

                    if (!c) {
                        //TODO: add ALOT more here
                        auto& map = *server.m_MapSpecificData.m_Map;
                        auto& sourceMapKey = list[0];
                        auto* sourceClient = server.getClientByUsername(pI.name);
                        auto& ships = map.getShips();
                        if (ships.count(sourceMapKey)) {
                            Ship* ship = ships.at(sourceMapKey);
                            if (ship) { //TODO: should not need this nullptr check
                                //health status
                                PacketHealthUpdate pOut2(*ship);
                                pOut2.PacketType = PacketType::Server_To_Client_Ship_Health_Update;
                                server.send_to_client(*sourceClient, pOut2);

                                //cloak status
                                PacketCloakUpdate pOut1(*ship);
                                pOut1.PacketType = PacketType::Server_To_Client_Ship_Cloak_Update;
                                server.send_to_client(*sourceClient, pOut1);

                                //target status
                                ship->setTarget(ship->getTarget(), true); //sends target packet info to the new guy

                                //anti cloak scan status
                                ShipSystemSensors* sensors = static_cast<ShipSystemSensors*>(ship->getShipSystem(ShipSystemType::Sensors));
                                if (sensors) {
                                    sensors->sendAntiCloakScanStatusPacket();
                                }
                            }
                        }
                    }else{
                        server.send_to_client(*c, pOut); //ask the client for info
                    }
                    break;
                }case PacketType::Client_To_Server_Collision_Event: {
                    PacketCollisionEvent& pI = *static_cast<PacketCollisionEvent*>(pp);
                    auto& map = *server.m_MapSpecificData.m_Map;
                    server.m_MapSpecificData.m_CollisionEntries.processCollision(pI, map);
                    break;
                }case PacketType::Client_To_Server_Anti_Cloak_Status: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Anti_Cloak_Status;
                    server.send_to_all_but_client(client, pOut);
                    break;
                }case PacketType::Client_To_Server_Projectile_Cannon_Impact: {
                    //just forward it
                    PacketProjectileImpact& pI = *static_cast<PacketProjectileImpact*>(pp);
                    PacketProjectileImpact pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Projectile_Cannon_Impact;
                    server.send_to_all(pOut);
                    break;
                }case PacketType::Client_To_Server_Projectile_Torpedo_Impact: {
                    //just forward it
                    PacketProjectileImpact& pI = *static_cast<PacketProjectileImpact*>(pp);
                    PacketProjectileImpact pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Projectile_Torpedo_Impact;
                    server.send_to_all(pOut);
                    break;

                }case PacketType::Client_To_Server_Periodic_Ping: {
                    //thanks for staying with us
                    client.m_Timeout = 0.0f;
                    break;
                }case PacketType::Client_To_Server_Client_Fired_Beams: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Client_Fired_Beams;
                    server.send_to_all(pOut);
                    break;
                }case PacketType::Client_To_Server_Client_Fired_Cannons: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Client_Fired_Cannons;
                    server.send_to_all(pOut);
                    break;
                }case PacketType::Client_To_Server_Client_Fired_Torpedos: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Client_Fired_Torpedos;
                    server.send_to_all(pOut);
                    break;
                }case PacketType::Client_To_Server_Client_Changed_Target: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Client_Changed_Target;
                    server.send_to_all_but_client(client, pOut);
                    break;
                }case PacketType::Client_To_Server_Successfully_Entered_Map: {
                    //client told me he entered, lets give his data to the other clients, and give him info about the current deep space anchors, and info about each other client

                    //when a client receives  PacketType::Server_To_Client_New_Client_Entered_Map, they add the new client ship to their map pool and send the server info about themselves

                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut1(pI);
                    client.m_MapKey = pI.name; //pI.name should be the right map key
                    pOut1.PacketType = PacketType::Server_To_Client_New_Client_Entered_Map;
                    server.send_to_all_but_client(client, pOut1);

                    auto list = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass, map

                    auto& map = *static_cast<Map*>(Resources::getScene(list[1]));
                    auto spawnPosition = map.getSpawnAnchor()->getPosition();
                    for (auto& anchor : map.getRootAnchor()->getChildren()) {
                        if (boost::contains(anchor.first, "Deepspace Anchor")) {
                            const auto anchorPosition = anchor.second->getPosition();
                            PacketMessage pOut2;
                            pOut2.PacketType = PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial;
                            pOut2.r = static_cast<float>(anchorPosition.x) - static_cast<float>(spawnPosition.x);
                            pOut2.g = static_cast<float>(anchorPosition.y) - static_cast<float>(spawnPosition.y);
                            pOut2.b = static_cast<float>(anchorPosition.z) - static_cast<float>(spawnPosition.z);
                            pOut2.data = anchor.first;
                            server.send_to_client(client, pOut2);
                        }
                    }
                    break;
                }case PacketType::Client_To_Server_Request_Anchor_Creation: {
                    server.m_MapSpecificData.internal_process_deepspace_anchor(pp);
                    break;
                }case PacketType::Client_To_Server_Ship_Health_Update: {
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
                    server.send_to_all_but_client(client, pOut);
                    break;
                }case PacketType::Client_To_Server_Ship_Physics_Update: {
                    //a client has sent the server it's physics information, lets forward it
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

                    auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                    const int teamNumber = stoi(info[2]);
                    if (teamNumber == -1) {
                        server.assignRandomTeam(pOut, client);
                    }
                    server.send_to_client(client, pOut);
                    break;
                }case PacketType::Client_To_Server_Chat_Message: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Server_To_Client_Chat_Message;
                    server.send_to_all(pOut);
                    break;
                }case PacketType::Client_To_Server_Request_Connection: {
                    const bool valid = server.isValidName(pIn.data);
                    server.assign_username_to_client(client, pIn.data);
                    Packet pOut;
                    if (valid) {
                        //a client wants to connect to the server
                        client.m_Validated = true;
                        pOut.PacketType = PacketType::Server_To_Client_Accept_Connection;
                        pOut.data = "";
                        for (auto& clientThread : server.m_Threads) {
                            for (auto& c : clientThread->m_Clients) {
                                if (!c.second->m_Username.empty() && c.second->m_Username != client.m_Username) {
                                    pOut.data += c.second->m_Username + ",";
                                }
                            }
                        }
                        pOut.data += std::to_string(client.m_ID) + ",";
                        if (!pOut.data.empty())
                            pOut.data.pop_back();
                        server.send_to_client(client, pOut);
                        std::cout << "Server: Approving: " + pIn.data + "'s connection" << std::endl;

                        //now send the client info about the gameplay mode, dont do this for the player client
                        if (client.m_Username != server.m_Core.m_Client->m_Username) {
                            auto& mode = *server.m_GameplayMode;
                            const auto info = mode.serialize();
                            PacketMessage pOut0;
                            pOut0.name = client.m_Username;
                            pOut0.data = info;
                            pOut0.PacketType = PacketType::Server_To_Client_Request_GameplayMode;
                            server.send_to_client(client, pOut0);
                        }

                        PacketMessage pOut1;
                        pOut1.name = client.m_Username;
                        pOut1.data = "";
                        pOut1.PacketType = PacketType::Server_To_Client_Client_Joined_Server;
                        server.send_to_all(pOut1);

                        PacketMessage pOut2;
                        Map* map = server.m_MapSpecificData.m_Map;
                        pOut2.name = map->name();
                        pOut2.data = map->allowedShipsSingleString();
                        pOut2.PacketType = PacketType::Server_To_Client_Map_Data;
                        server.send_to_client(client, pOut2);
                    }else{
                        pOut.PacketType = PacketType::Server_To_Client_Reject_Connection;
                        std::cout << "Server: Rejecting: " + pIn.data + "'s connection" << std::endl;
                        server.send_to_client(client, pOut);
                        server.completely_remove_client(client);
                    }
                    break;
                }case PacketType::Client_To_Server_Request_Disconnection: {
                    std::cout << "Server: Removing " + client.m_Username + " from the server" << std::endl;
                    const auto& client_ip = client.m_TcpSocket->ip();
                    const auto& client_port = client.m_TcpSocket->remotePort();
                    const auto& client_address = client_ip + " " + to_string(client_port);

                    PacketMessage pOut1;
                    pOut1.name = client.m_Username;
                    pOut1.data = "";
                    pOut1.PacketType = PacketType::Server_To_Client_Client_Left_Server;
                    server.send_to_all(pOut1);

                    client.disconnect();
                    server.m_Mutex.lock();
                    server.m_ClientsToBeDisconnected.push(client_address);
                    server.m_Mutex.unlock();
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(pp);
    }
}
void Server::assignRandomTeam(PacketMessage& packet_out, ServerClient& client) {
    //assign the player a team number
    auto& teams = m_GameplayMode->getTeams();
    Team* chosen = nullptr;
    size_t minVal = -1;
    for (auto& team : teams) {
        const auto numberOfPlayers = team.second->getNumberOfPlayersOnTeam();
        if (numberOfPlayers < minVal) {
            minVal = numberOfPlayers;
            chosen = team.second;
        }
    }
    if (chosen) {
        Team& team = *chosen;
        boost::replace_all(packet_out.data, ",-1", "," + team.getTeamNumberAsString());
        teams.at(team.getTeamNumber())->addPlayerToTeam(client.m_MapKey);
    }
}

ServerClient* Server::getClientByUsername(const string& username) {
    ServerClient* ret = nullptr;
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second->m_Username == username) {
                return client.second;
            }
        }
    }
    return ret;
}
ServerClient* Server::getClientByMapKey(const string& MapKey) {
    ServerClient* ret = nullptr;
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second->m_MapKey == MapKey) {
                return client.second;
            }
        }
    }
    return ret;
}

const bool Server::isValidName(const string& name) const {
    if (name.empty())
        return false;
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second->m_Username == name) {
                return false;
            }
        }
    }
    return true;
}

void Server::assign_username_to_client(ServerClient& client, const string& username) {
    string final_username = username;
    unsigned int count = 0;
    for (auto& thread : m_Threads) {
        for (auto& server_client_ptr : thread->m_Clients) {
            if (server_client_ptr.second->m_Username == final_username) {
                ++count;
                final_username = username + "_" + to_string(count);
            }
        }
    }
    client.m_Username = final_username;
    client.m_MapKey = final_username;
}

#pragma region sending / recieving
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
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                auto status = client.second->send(packet);
            }
        }
    }
}
void Server::send_to_all_but_client(ServerClient& c, sf::Packet& packet) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                auto status = client.second->send(packet);
            }
        }
    }
}
void Server::send_to_all_but_client(ServerClient& c, const void* data, size_t size) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                auto status = client.second->send(data, size);
            }
        }
    }
}
void Server::send_to_all_but_client(ServerClient& c, const void* data, size_t size, size_t& sent) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                auto status = client.second->send(data, size, sent);
            }
        }
    }
}
void Server::send_to_all(Packet& packet) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            auto status = client.second->send(packet);
        }
    }
}
void Server::send_to_all(sf::Packet& packet) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            auto status = client.second->send(packet);
        }
    }
}
void Server::send_to_all(const void* data, size_t size) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            auto status = client.second->send(data, size);
        }
    }
}
void Server::send_to_all(const void* data, size_t size, size_t& sent) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            auto status = client.second->send(data, size, sent);
        }
    }
}

void Server::send_to_all_but_client_udp(ServerClient& c, Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                const auto clientIP = client.second->m_IP;
                const auto clientID = client.second->m_ID;
                auto status = m_UdpSocket->send(m_port + 1 + clientID, sf_packet, clientIP);
            }
        }
    }
}
void Server::send_to_all_but_client_udp(ServerClient& c, sf::Packet& packet) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                const auto clientIP = client.second->m_IP;
                const auto clientID = client.second->m_ID;
                auto status = m_UdpSocket->send(m_port + 1 + clientID, packet, clientIP);
            }
        }
    }
}
void Server::send_to_all_but_client_udp(ServerClient& c, const void* data, size_t size) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            if (client.second != &c) {
                const auto clientIP = client.second->m_IP;
                const auto clientID = client.second->m_ID;
                auto status = m_UdpSocket->send(m_port + 1 + clientID, data, size, clientIP);
            }
        }
    }
}

void Server::send_to_all_udp(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            const auto clientIP = client.second->m_IP;
            const auto clientID = client.second->m_ID;
            auto status = m_UdpSocket->send(m_port + 1 + clientID, sf_packet, clientIP);
        }
    }
}
void Server::send_to_all_udp(sf::Packet& packet) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            const auto clientIP = client.second->m_IP;
            const auto clientID = client.second->m_ID;
            auto status = m_UdpSocket->send(m_port + 1 + clientID, packet, clientIP);
        }
    }
}
void Server::send_to_all_udp(const void* data, size_t size) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            const auto clientIP = client.second->m_IP;
            const auto clientID = client.second->m_ID;
            auto status = m_UdpSocket->send(m_port + 1 + clientID, data, size, clientIP);
        }
    }
}
const sf::Socket::Status Server::receive_udp(sf::Packet& packet) {
    const auto status = m_UdpSocket->receive(packet);
    return status;
}
const sf::Socket::Status Server::receive_udp(void* data, size_t size, size_t& received) {
    const auto status = m_UdpSocket->receive(data, size, received);
    return status;
}
#pragma endregion

#pragma endregion
#include "Server.h"
#include "Packet.h"
#include "Client.h"
#include "../Core.h"
#include "../Menu.h"
#include "../Helper.h"
#include "../ResourceManifest.h"
#include "../gui/specifics/ServerLobbyChatWindow.h"
#include "../gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "../modes/GameplayMode.h"

#include "../map/Map.h"
#include "../map/Anchor.h"
#include "../teams/Team.h"
#include "../modes/GameplayMode.h"
#include "../ships/Ships.h"

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemSensors.h"
#include "../ships/shipSystems/ShipSystemHull.h"

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

const double TIMER_DEEP_SPACE_ANCHOR_SPAM = 1.0;

#pragma region RespawningShips
ShipRespawning::ShipRespawning(Server& server):m_Server(server){

}
ShipRespawning::~ShipRespawning() {

}
void ShipRespawning::processShip(const string& shipMapKey, const string& shipClass, const string& closest_spawn_anchor) {
    if (!m_Ships.count(shipMapKey)) {
        m_Ships.emplace(shipMapKey, make_tuple(shipClass, closest_spawn_anchor, Ships::Database[shipClass].RespawnTime));
    }else {
        //this should really not happen if cleanup() is implemented, but just to be safe...
        m_Ships.erase(shipMapKey);
        m_Ships.emplace(shipMapKey, make_tuple(shipClass, closest_spawn_anchor, Ships::Database[shipClass].RespawnTime));
    }
    PacketMessage pOut;
    pOut.PacketType = PacketType::Server_To_Client_Notify_Ship_Of_Impending_Respawn;
    pOut.name = shipMapKey;
    pOut.r = static_cast<float>(Ships::Database[shipClass].RespawnTime);
    m_Server.send_to_client(*m_Server.getClientByMapKey(shipMapKey), pOut);

    //std::cout << "Processing Ship: " << shipMapKey << " (" << shipClass << ") at closest anchor: " << closest_spawn_anchor << ", for duration: " << to_string(Ships::Database[shipClass].RespawnTime) << "\n";
}
void ShipRespawning::cleanup() {
    for (auto it = m_Ships.begin(); it != m_Ships.end();) {
        auto& tuple = it->second;
        if (std::get<2>(tuple) <= 0.0) {
            it = m_Ships.erase(it);
        }else {
            it++;
        }
    }
}
void ShipRespawning::update(const double& dt) {
    for (auto& it : m_Ships) {
        auto& tuple = it.second;
        auto& respawn_time = std::get<2>(tuple);
        if (respawn_time > 0.0) {
            respawn_time -= dt;
            if (respawn_time <= 0.0) {
                respawn_time = 0.0;

                //TODO: calculate respawn position based on proximity to other ships / stations etc
                auto x = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
                auto y = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
                auto z = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
                auto respawnPosition = glm::vec3(x, y, z);

                PacketMessage pOut;
                pOut.PacketType = PacketType::Server_To_Client_Notify_Ship_Of_Respawn;
                pOut.name = it.first; //mapkey
                pOut.r = respawnPosition.x;
                pOut.g = respawnPosition.y;
                pOut.b = respawnPosition.z;
                pOut.data = std::get<1>(tuple); //closest_spawn_anchor
                //pOut.data += "," + std::get<0>(tuple); //shipclass
                m_Server.send_to_all(pOut);

                //std::cout << "Processing Ship: " << it.first << " (" << std::get<0>(tuple) << ") may now respawn\n";
            }
        }
    }
    cleanup();
}
#pragma endregion

#pragma region CollisionEntries
CollisionEntries::CollisionEntries(Server& server):m_Server(server) {

}
CollisionEntries::~CollisionEntries() {

}

void CollisionEntries::processCollision(const PacketCollisionEvent& packet_in, Map& map) {
    auto info = Helper::SeparateStringByCharacter(packet_in.data, ',');
    string key1 = info[0] + "|" + info[1];
    string key2 = info[1] + "|" + info[0];

    const bool is_key_1 = m_CollisionPairs.count(key1);
    const bool is_key_2 = m_CollisionPairs.count(key2);
    
    auto lambda_send = [&]() {
        PacketCollisionEvent pOut(packet_in);
        pOut.PacketType = PacketType::Server_To_Client_Collision_Event;
        m_Server.send_to_all(pOut);
    };

    if (!is_key_1 && !is_key_2) {
        m_CollisionPairs.emplace(key1, 2.0);
        lambda_send();
    }else{
        if (is_key_1) {
            if (m_CollisionPairs[key1] <= 0.0) {
                m_CollisionPairs[key1] = 2.0;
                lambda_send();
            }
        }else if (is_key_2) {
            if (m_CollisionPairs[key2] <= 0.0) {
                m_CollisionPairs[key2] = 2.0;
                lambda_send();
            }
        }
    }
}
void CollisionEntries::cleanup() {
    for (auto it = m_CollisionPairs.begin(); it != m_CollisionPairs.end();) {
        if (it->second <= 0.0) {
            it = m_CollisionPairs.erase(it);
        }else {
            it++;
        }
    }
}
void CollisionEntries::update(const double& dt) {
    for (auto& it : m_CollisionPairs) {
        if (it.second > 0.0) {
            it.second -= dt;
            if (it.second <= 0.0) {
                it.second = 0.0;
            }
        }
    }
}
#pragma endregion

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

Server::Server(Core& core, const unsigned int& port, const string& ipRestriction) :m_Core(core), m_CollisionEntries(*this), m_RespawningShips(*this) {
    m_GameplayMode                     = nullptr;
    m_OwnerClient                      = nullptr;
    m_port                             = port;
    m_listener                         = new ListenerTCP(port, ipRestriction);
    m_DeepspaceAnchorTimer             = 0.0;
    m_PingTime                         = 0.0;
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
    const auto res = Resources::deleteScene(m_MapName);
    if (res) {
        Resources::setCurrentScene("Menu");
    }
    return res;
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
    m_MapName = mapname;
    const sf::Socket::Status status = listener.listen();

    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        auto* thread = new ServerClientThread();
        m_Threads.push_back(thread);
    }

    if (status == sf::Socket::Status::Done) {
        listener.setBlocking(false);
        m_Active.store(1, std::memory_order_relaxed);
        return true;
    }
    return false;
}
const bool Server::startupMap(GameplayMode& mode) {
    m_GameplayMode = &mode;
    Map* map = static_cast<Map*>(Resources::getScene(m_MapName));
    if (!map) {
        map = new Map(mode , *m_Core.m_Client, m_MapName, ResourceManifest::BasePath + "data/Systems/" + m_MapName + ".txt");
        map->m_IsServer = true;
        return true;
    }
    return false;
}
void Server::update_server_entities(const double& dt) {
    if (m_Core.gameState() != GameState::Game)
        return;

    m_PingTime += dt;
    if (m_PingTime > PHYSICS_PACKET_TIMER_LIMIT) {
        //keep pinging the server, sending your ship physics info
        auto& map = *static_cast<Map*>(Resources::getScene(m_MapName));

        Anchor* finalAnchor = nullptr;
        vector<string> list;
        for (auto& ship_ptr : map.getShipsNPCControlled()) {
            Ship& ship = *ship_ptr.second;
            finalAnchor = map.getRootAnchor();
            list = map.getClosestAnchor(nullptr, &ship);
            for (auto& closest : list) {
                finalAnchor = finalAnchor->getChildren().at(closest);
            }
            PacketPhysicsUpdate p(ship, map, finalAnchor, list, ship.getName());
            p.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;
            send_to_all_but_client_udp(*m_OwnerClient, p);
        }
        /*
        auto playerPos = glm::vec3(playerShip.getPosition());
        auto nearestAnchorPos = glm::vec3(finalAnchor->getPosition());
        double distFromMeToNearestAnchor = static_cast<double>(glm::distance2(nearestAnchorPos, playerPos));

        if (distFromMeToNearestAnchor > DISTANCE_CHECK_NEAREST_ANCHOR) {
            for (auto& otherShips : map.getShips()) {
                if (otherShips.first != playerShip.getName()) {
                    auto otherPlayerPos = glm::vec3(otherShips.second->getPosition());
                    auto distFromMeToOtherPlayerSq = glm::distance2(otherPlayerPos, playerPos);
                    const auto calc = (distFromMeToNearestAnchor - DISTANCE_CHECK_NEAREST_ANCHOR) * 0.5f;
                    if (distFromMeToOtherPlayerSq < glm::max(calc, DISTANCE_CHECK_NEAREST_OTHER_PLAYER)) {
                        const glm::vec3 midpoint = Math::midpoint(otherPlayerPos, playerPos);

                        PacketMessage pOut;
                        pOut.PacketType = PacketType::Client_To_Server_Request_Anchor_Creation;
                        pOut.r = midpoint.x - nearestAnchorPos.x;
                        pOut.g = midpoint.y - nearestAnchorPos.y;
                        pOut.b = midpoint.z - nearestAnchorPos.z;
                        pOut.data = "";
                        pOut.data += to_string(list.size());
                        for (auto& closest : list) {
                            pOut.data += "," + closest;
                        }
                        //we want to create an anchor at r,g,b (the midpoint between two nearby ships), we send the nearest valid anchor as a reference
                        client.send(pOut);
                        break;
                    }
                }
            }
        }
        */
        m_PingTime = 0.0;
    }
}
void Server::update(Server* thisServer, const double& dt) {
    auto& server = *thisServer;
    const auto server_active = server.m_Active.load(std::memory_order_relaxed);
    if (server_active == 1) {
        server.m_DeepspaceAnchorTimer += dt;
        updateAcceptNewClients(server);
        server.update_server_entities(dt);
        server.m_CollisionEntries.update(dt);
        server.m_RespawningShips.update(dt);
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
                        return;
                    }
                }else{
                    client.m_Timeout += dt;
                    if (client.m_Timeout > SERVER_CLIENT_TIMEOUT) {
                        std::cout << "Client: " << client.m_Username << " has timed out, disconnecting..." << std::endl;
                        client.disconnect();
                        return;
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

                    auto& map = *static_cast<Map*>(Resources::getScene(server.m_MapName));
                    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    if (map.hasShipPlayer(pI.name)) {
                        //this is a player ship, give it permission to respawn, currently NPC's do not respawn
                        server.m_RespawningShips.processShip(pI.name, list[0], map.getClosestSpawnAnchor());
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
                        auto& map = *static_cast<Map*>(Resources::getScene(server.m_MapName));
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
                    auto& map = *static_cast<Map*>(Resources::getScene(server.m_MapName));
                    server.m_CollisionEntries.processCollision(pI, map);
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
                    if (server.m_DeepspaceAnchorTimer > TIMER_DEEP_SPACE_ANCHOR_SPAM) {
                        //just forward it
                        PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                        PacketMessage pOut(pI);
                        pOut.PacketType = PacketType::Server_To_Client_Anchor_Creation;
                        server.send_to_all(pOut);
                        server.m_DeepspaceAnchorTimer = 0.0;
                    }
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
                        pOut2.name = server.m_MapName;
                        Map* map = static_cast<Map*>(Resources::getScene(server.m_MapName));
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
        boost::replace_all(packet_out.data, ",-1", "," + chosen->getTeamNumberAsString());
        teams.at(chosen->getTeamNumber())->addPlayerToTeam(client.m_Username);
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

#pragma endregion
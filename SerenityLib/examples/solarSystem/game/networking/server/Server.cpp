#include "Server.h"
#include "ServerMapSpecificData.h"
#include "../packets/Packets.h"
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

#include <core/engine/system/Engine.h>
#include <core/engine/utils/Engine_Debugging.h>

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::Networking;

ServerHostData          Server::SERVER_HOST_DATA;
Database                Server::DATABASE;
Server::PersistentInfo  Server::PERSISTENT_INFO;

#pragma region ServerHostData

ServerHostData::ServerHostData() {
    setGameplayModeType(GameplayModeType::FFA);
    setServerPort(55000);
    setCurrentLobbyTimeInSeconds(0.0);
    setLobbyTimeInSeconds(0.0);
    setMatchDurationInMinutes(15); //15 mins
}
ServerHostData::~ServerHostData() {

}

void ServerHostData::setCurrentLobbyTimeInSeconds(const double& seconds) {
    m_CurrentLobbyTimeInSeconds = (seconds);
}
const double& ServerHostData::getCurrentLobbyTime() const {
    return m_CurrentLobbyTimeInSeconds;
}


void ServerHostData::setLobbyTimeInSeconds(const double& seconds) {
    m_LobbyTimeInSeconds = (seconds);
}
const double& ServerHostData::getLobbyTime() const {
    return m_LobbyTimeInSeconds;
}


const MapEntryData& ServerHostData::getMapChoice() const {
    return m_CurrentMapChoice;
}
void ServerHostData::setMatchDurationInMinutes(const unsigned int& minutes) {
    m_ExpectedMatchDurationInSeconds = 60 * minutes;
}
void ServerHostData::setServerPort(const unsigned short& port) {
    m_ServerPort = port;
}
const unsigned short& ServerHostData::getServerPort() const {
    return m_ServerPort;
}
void ServerHostData::setMatchDurationInSeconds(const unsigned int& seconds) {
    m_ExpectedMatchDurationInSeconds = seconds;
}
void ServerHostData::setMapChoice(const MapEntryData& choice) {
    m_CurrentMapChoice = choice;
}
const bool ServerHostData::addTeam(Team& team) {
    return m_GameplayMode.addTeam(team);
}
const GameplayModeType::Mode& ServerHostData::getGameplayModeType() const {
    return m_GameplayMode.m_GameplayModeType;
}
void ServerHostData::setMaxAmountOfPlayers(const unsigned int& players) {
    m_GameplayMode.setMaxAmountOfPlayers(players);
}
void ServerHostData::setGameplayModeType(const GameplayModeType::Mode& mode) {
    m_GameplayMode.setGameplayMode(mode);
}
const GameplayMode& ServerHostData::getGameplayMode() const {
    return m_GameplayMode;
}
unordered_set<string>& ServerHostData::getAllowedShips() {
    return m_GameplayMode.m_AllowedShipClasses;
}
void ServerHostData::setAllowedShips(const vector<string>& allowed_ships) {
    unordered_set<string> ret;
    for (auto& ship_class : allowed_ships) {
        if (!ret.count(ship_class)) {
            ret.insert(ship_class);
        }
    }
    m_GameplayMode.m_AllowedShipClasses = ret;
}
void ServerHostData::setAllowedShips(const unordered_set<string>& allowed_ships) {
    m_GameplayMode.m_AllowedShipClasses = allowed_ships;
}
const string& ServerHostData::getGameplayModeString() const {
    return GameplayMode::GAMEPLAY_TYPE_ENUM_NAMES[m_GameplayMode.m_GameplayModeType];
}

#pragma endregion

#pragma region ServerClient

ServerClient::ServerClient(const string& hash, Server& server, Core& core, Engine::Networking::SocketTCP& socket) : m_Core(core), m_Server(server) {
    m_TcpSocket = &socket;
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
    m_Thread = NEW std::future<bool>(std::move(std::async(std::launch::async, updateClientThread, std::ref(*this))));
    */    
}
ServerClientThread::~ServerClientThread() {
    m_Shutdowned = true;
    m_Active.store(0, std::memory_order_relaxed);
    SAFE_DELETE_FUTURE(m_Thread);
    SAFE_DELETE_MAP(m_Clients);

}
#pragma endregion

#pragma region PersistentInfo

Server::PersistentInfo::PersistentInfo() {
    clearInfo();
}
AuthenticationLayer& Server::getAuthenticationLayer() {
    return m_AuthenticationLayer;
}
const string& Server::PersistentInfo::getOwnerName() const {
    return m_OwnerName;
}
const string& Server::PersistentInfo::getServerName() const {
    return m_ServerName;
}
const unsigned short& Server::PersistentInfo::getPort() const {
    return m_Port;
}
bool Server::PersistentInfo::operator==(const bool& rhs) const {
    if (rhs == true) {
        return (!m_ServerName.empty() && !m_OwnerName.empty() && m_Port != 0) ? true : false;
    }else{
        return (!m_ServerName.empty() && !m_OwnerName.empty() && m_Port != 0) ? false : true;
    }
    return false;
}
Server::PersistentInfo::operator bool() const {
    return (!m_ServerName.empty() && !m_OwnerName.empty() && m_Port != 0) ? true : false;
}
void Server::PersistentInfo::setInfo(const string& serverName, const unsigned short& port, const string& ownerName) {
    m_ServerName = serverName;
    m_Port       = port;
    m_OwnerName  = ownerName;
}
void Server::PersistentInfo::clearInfo() {
    m_ServerName = "";
    m_OwnerName  = "";
    m_Port       = 0;
}

#pragma endregion

#pragma region Server

Server::Server(Core& core, const unsigned int& port, const string& ipRestriction) : m_Core(core), m_MapSpecificData(*this), m_AuthenticationLayer(*this){
    m_OwnerClient                      = nullptr;
    m_Port                             = port;
    m_TCPListener                      = NEW ListenerTCP(port, ipRestriction);
    m_UdpSocket                        = NEW SocketUDP(port, ipRestriction);
    m_UdpSocket->setBlocking(false);
    m_UdpSocket->bind();
    m_Active.store(0, std::memory_order_relaxed);
}
Server::~Server() {
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
    Sleep(1000); //messy
    m_UdpSocket->unbind();
    m_TCPListener->close();
    if (destructor) {
        SAFE_DELETE_VECTOR(m_Threads);
        SAFE_DELETE(m_TCPListener);
        SAFE_DELETE(m_UdpSocket);
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

const bool Server::startup() {
    auto& listener = *m_TCPListener;
    const sf::Socket::Status status = listener.listen();

    if (m_Threads.size() == 0) {
        for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
            m_Threads.push_back(NEW ServerClientThread());
        }
    }
    if (status == sf::Socket::Status::Done) {
        listener.setBlocking(false);
        m_Active.store(1, std::memory_order_relaxed);
        return true;
    }
    return false;
}
const bool Server::startupMap(const MapEntryData& map_data) {
    m_MapSpecificData.m_Map  = static_cast<Map*>(Resources::getScene(map_data.map_name));
    if (!m_MapSpecificData.m_Map) {
        auto* map = NEW Map(const_cast<GameplayMode&>(Server::SERVER_HOST_DATA.getGameplayMode()), *m_Core.m_Client, map_data.map_name, map_data.map_file_path);
        m_MapSpecificData.m_Map = static_cast<Map*>(Resources::getScene(map_data.map_name));
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
        server.m_AuthenticationLayer.update(dt);
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
            priv::threading::addJobRef(updateClientThread, *thread);
        }
        //epriv::threading::waitForAll();
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        //updateRemoveDisconnectedClients(server);
    }
}
//Listener thread (NOT multithreaded)
void Server::updateAcceptNewClients(Server& server) {
    if (server.m_TCPListener) {
        sf::TcpSocket* sf_client = NEW sf::TcpSocket();
        auto& sfClient = *sf_client;
        const auto& status = server.m_TCPListener->accept(sfClient);
        if (status == sf::Socket::Status::Done) {
            const auto& client_ip      = sfClient.getRemoteAddress().toString();
            const auto& client_port    = sfClient.getRemotePort();
            const auto& client_address = client_ip + " " + to_string(client_port);

            //initial connection established: start the authentication process
            server.m_AuthenticationLayer.create_instance(client_address, sf_client);
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
        auto basePacket_Ptr = std::unique_ptr<Packet>(std::move(Packet::getPacket(sf_packet_udp)));
        const auto valid = basePacket_Ptr->validate(sf_packet_udp);
        if (basePacket_Ptr && valid) {
            Packet& pIn = *basePacket_Ptr.get();
            switch (basePacket_Ptr->PacketType) {
                case PacketType::Client_To_Server_Ship_Physics_Update: {
                    //a client has sent the server it's physics information, lets forward it
                    PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;

                    //get the client who sent it. TODO: this is sort of expensive
                    ServerClient* client_ptr = getClientByUsername(pI.player_username);
                    if (client_ptr) { //this works...
                        auto& client     = *client_ptr;
                        client.m_Timeout = 0.0f;
                        send_to_all_but_client_udp(client, pI);
                    }
                    break;
                }default: {
                    break;
                }
            }
        }
    }
}
//NOT multithreaded
void Server::completely_remove_client(ServerClient& client) {
    if (m_OwnerClient) {
        if (m_OwnerClient->m_MapKey == client.m_MapKey) {
            m_OwnerClient = nullptr;
        }
    }
    m_MapSpecificData.removeShip(client.m_MapKey);
    for (auto& clientThread : m_Threads) {
        for (auto& _client : clientThread->m_Clients) {
            string username_cpy = _client.second->m_Username;
            if (client.m_MapKey == _client.second->m_MapKey) {
                std::cout << "Client: " << username_cpy << " - has been completely removed from the server" << std::endl;
                //std::lock_guard<std::mutex> lock_guard(m_Mutex);
                clientThread->m_Clients.erase(_client.first);
                if (clientThread->m_Clients.size() == 0) {
                    clientThread->m_Active.store(0, std::memory_order_relaxed);
                }
                return;
            }
        }
    }
}
//NOT multithreaded
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
        auto basePacket_Ptr = std::unique_ptr<Packet>(std::move(Packet::getPacket(sf_packet)));
        const auto valid = basePacket_Ptr->validate(sf_packet);
        if (basePacket_Ptr && valid) {
            Packet& pIn = *basePacket_Ptr;
            client.m_Timeout = 0.0f;
            // Data extracted successfully...
            switch (pIn.PacketType) {
                case PacketType::Client_To_Server_Ship_Was_Just_Destroyed: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Ship_Was_Just_Destroyed;
                    server.send_to_all_but_client(client, pI);

                    auto& map = *server.m_MapSpecificData.m_Map;
                    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    if (map.hasShipPlayer(pI.name)) {
                        //this is a player ship, give it permission to respawn, currently NPC's do not respawn
                        server.m_MapSpecificData.m_RespawningShips.processShip(pI.name, list[0], map.getClosestSpawnAnchor());
                    }
                    break;
                }case PacketType::Client_To_Server_Request_Ship_Current_Info: {
                    PacketShipInfoRequest& pI = *static_cast<PacketShipInfoRequest*>(&pIn);
                    pI.PacketType   = PacketType::Server_To_Client_Request_Ship_Current_Info;

                    auto* requested_ship_client = server.getClientByUsername(pI.requested_ship_username);

                    if (!requested_ship_client) {
                        //TODO: add ALOT more here
                        auto& map                            = *server.m_MapSpecificData.m_Map;
                        auto* client_that_requested_the_info = server.getClientByUsername(pI.ship_that_wants_info_key);
                        auto& ships                          = map.getShips();
                        if (ships.count(pI.requested_ship_key)) {
                            Ship* requested_ship     = ships.at(pI.requested_ship_key);
                            if (requested_ship && client_that_requested_the_info) { //TODO: should not need this nullptr check
                                //health status
                                PacketHealthUpdate packetHealth(*requested_ship);
                                packetHealth.PacketType = PacketType::Server_To_Client_Ship_Health_Update;
                                server.send_to_client(*client_that_requested_the_info, packetHealth);

                                //cloak status
                                PacketCloakUpdate packetCloak(*requested_ship);
                                packetCloak.PacketType = PacketType::Server_To_Client_Ship_Cloak_Update;
                                server.send_to_client(*client_that_requested_the_info, packetCloak);

                                //target status
                                requested_ship->setTarget(requested_ship->getTarget(), true); //sends target packet info to the new guy

                                //anti cloak scan status
                                ShipSystemSensors* sensors = static_cast<ShipSystemSensors*>(requested_ship->getShipSystem(ShipSystemType::Sensors));
                                if (sensors) {
                                    sensors->sendAntiCloakScanStatusPacket();
                                }
                            }
                        }
                    }else{
                        server.send_to_client(*requested_ship_client, pI); //ask the client for info
                    }
                    break;
                }case PacketType::Client_To_Server_Collision_Event: {
                    PacketCollisionEvent& pI = *static_cast<PacketCollisionEvent*>(&pIn);
                    auto& map = *server.m_MapSpecificData.m_Map;
                    server.m_MapSpecificData.m_CollisionEntries.processCollision(pI, map);
                    break;
                }case PacketType::Client_To_Server_Anti_Cloak_Status: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Anti_Cloak_Status;
                    server.send_to_all_but_client(client, pI);
                    break;
                }case PacketType::Client_To_Server_Projectile_Cannon_Impact: {
                    //just forward it
                    PacketProjectileImpact& pI = *static_cast<PacketProjectileImpact*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Projectile_Cannon_Impact;
                    server.send_to_all(pI);
                    break;
                }case PacketType::Client_To_Server_Projectile_Torpedo_Impact: {
                    //just forward it
                    PacketProjectileImpact& pI = *static_cast<PacketProjectileImpact*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Projectile_Torpedo_Impact;
                    server.send_to_all(pI);
                    break;
                }case PacketType::Client_To_Server_Client_Fired_Beams: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Client_Fired_Beams;
                    server.send_to_all(pI);
                    break;
                }case PacketType::Client_To_Server_Client_Fired_Cannons: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Client_Fired_Cannons;
                    server.send_to_all(pI);
                    break;
                }case PacketType::Client_To_Server_Client_Fired_Torpedos: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Client_Fired_Torpedos;
                    server.send_to_all(pI);
                    break;
                }case PacketType::Client_To_Server_Client_Changed_Target: {
                    //just forward it
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Client_Changed_Target;
                    server.send_to_all_but_client(client, pI);
                    break;
                }case PacketType::Client_To_Server_Successfully_Entered_Map: {
                    //client told me he entered, lets give his data to the other clients, and give him info about the current deep space anchors, and info about each other client

                    //when a client receives  PacketType::Server_To_Client_New_Client_Entered_Map, they add the new client ship to their map pool and send the server info about themselves

                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    client.m_MapKey = pI.name; //pI.name should be the right map key
                    pI.PacketType = PacketType::Server_To_Client_New_Client_Entered_Map;
                    server.send_to_all_but_client(client, pI);

                    auto list = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass, map

                    auto& map = *static_cast<Map*>(Resources::getScene(list[1]));
                    auto spawnPosition = map.getSpawnAnchor()->getPosition();
                    for (auto& anchor_itr : map.getRootAnchor()->getChildren()) {
                        auto& anchor_name = anchor_itr->getName();
                        if (boost::contains(anchor_name, "Deepspace Anchor")) {
                            const auto anchorPosition = anchor_itr->getPosition();
                            PacketMessage pOut2;
                            pOut2.PacketType = PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial;
                            pOut2.r = static_cast<float>(anchorPosition.x) - static_cast<float>(spawnPosition.x);
                            pOut2.g = static_cast<float>(anchorPosition.y) - static_cast<float>(spawnPosition.y);
                            pOut2.b = static_cast<float>(anchorPosition.z) - static_cast<float>(spawnPosition.z);
                            pOut2.data = anchor_name;
                            server.send_to_client(client, pOut2);
                        }
                    }
                    break;
                }case PacketType::Client_To_Server_Request_Anchor_Creation: {
                    server.m_MapSpecificData.internal_process_deepspace_anchor(&pIn);
                    break;
                }case PacketType::Client_To_Server_Ship_Health_Update: {
                    //just forward it
                    PacketHealthUpdate& pI = *static_cast<PacketHealthUpdate*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Ship_Health_Update;
                    server.send_to_all_but_client(client, pI);
                    break;
                }case PacketType::Client_To_Server_Ship_Cloak_Update: {
                    //a client has sent us it's cloaking information, forward it
                    PacketCloakUpdate& pI = *static_cast<PacketCloakUpdate*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Ship_Cloak_Update;
                    server.send_to_all_but_client(client, pI);
                    break;
                }case PacketType::Client_To_Server_Ship_Physics_Update: {
                    //a client has sent the server it's physics information, lets forward it
                    PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Ship_Physics_Update;
                    server.send_to_all_but_client(client, pI);
                    break;
                }case PacketType::Client_To_Server_Request_Map_Entry: {
                    //ok client, you told me you want in, lets approve you -> just bouncing
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Approve_Map_Entry;

                    auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                    const auto& teamNumber = info[2];
                    string res = "-1";
                    if (teamNumber == "-1") {
                        res = server.assignRandomTeam(pI, client);
                    }
                    info[2] = res;

                    //choose random ship if applicable
                    if (info[0] == "NULL") {
                        auto& allowed_ships = Server::SERVER_HOST_DATA.getAllowedShips();
                        const int rand_index = Helper::GetRandomIntFromTo(0, allowed_ships.size() - 1);
                        string rand_ship = "";

                        size_t i = 0;
                        for (auto& itr : allowed_ships) {
                            if (i == rand_index) {
                                rand_ship = itr;
                                break;
                            }
                            ++i;
                        }
                        info[0] = rand_ship;
                    }
                    pI.data = Helper::Stringify(info, ',');

                    server.send_to_client(client, pI);
                    break;
                }case PacketType::Client_To_Server_Chat_Message: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(&pIn);
                    pI.PacketType = PacketType::Server_To_Client_Chat_Message;
                    server.send_to_all(pI);
                    break;
                }case PacketType::Client_To_Server_Request_Disconnection: {
                    std::cout << "Server: Removing " + client.m_Username + " from the server (after requesting a disconnection)" << std::endl;
                    const auto& client_ip      = client.m_TcpSocket->ip();
                    const auto& client_port    = client.m_TcpSocket->remotePort();
                    const auto& client_address = client_ip + " " + to_string(client_port);

                    PacketMessage packetClientLeftServer;
                    packetClientLeftServer.name = client.m_Username;
                    packetClientLeftServer.PacketType = PacketType::Server_To_Client_Client_Left_Server;
                    server.send_to_all(packetClientLeftServer);

                    client.disconnect();
                    server.completely_remove_client(client);
                    break;
                }default: {
                    break;
                }
            }
        }
        //delete(basePacket_Ptr);
    }
}
string Server::assignRandomTeam(PacketMessage& packet_out, ServerClient& client) {
    //assign the player a team number
    auto& teams = const_cast<GameplayMode&>(Server::SERVER_HOST_DATA.getGameplayMode()).getTeams();
    Team* chosen = nullptr;
    size_t minVal = -1;
    for (auto& team : teams) {
        const auto numberOfPlayers = team.second.getNumberOfPlayersOnTeam();
        if (numberOfPlayers < minVal) {
            minVal = numberOfPlayers;
            chosen = &team.second;
        }
    }
    if (chosen) {
        Team& team = *chosen;
        const auto& team_num_as_str = team.getTeamNumberAsString();
        boost::replace_all(packet_out.data, ",-1", "," + team_num_as_str);
        team.addPlayerToTeam(client.m_MapKey);
        return team_num_as_str;
    }
    return "-1";
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
    client.m_MapKey   = final_username;
}
ServerClientThread* Server::getNextAvailableClientThread() {
    ServerClientThread* leastThread = nullptr;
    for (auto& clientThread : m_Threads) {
        if (!leastThread || (leastThread && leastThread->m_Clients.size() < clientThread->m_Clients.size())) {
            leastThread = clientThread;
        }
    }
    return leastThread;
}

#pragma region sending / recieving
const sf::Socket::Status Server::send_to_client(ServerClient& serverClient, Packet& packet) {
    return serverClient.send(packet);
}
const sf::Socket::Status Server::send_to_client(ServerClient& serverClient, sf::Packet& packet) {
    return serverClient.send(packet);
}
const sf::Socket::Status Server::send_to_client(ServerClient& serverClient, const void* data, size_t size) {
    return serverClient.send(data, size);
}
const sf::Socket::Status Server::send_to_client(ServerClient& serverClient, const void* data, size_t size, size_t& sent) {
    return serverClient.send(data, size, sent);
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
                auto status = m_UdpSocket->send(m_Port + 1 + clientID, sf_packet, clientIP);
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
                auto status = m_UdpSocket->send(m_Port + 1 + clientID, packet, clientIP);
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
                auto status = m_UdpSocket->send(m_Port + 1 + clientID, data, size, clientIP);
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
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, sf_packet, clientIP);
        }
    }
}
void Server::send_to_all_udp(sf::Packet& packet) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            const auto clientIP = client.second->m_IP;
            const auto clientID = client.second->m_ID;
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, packet, clientIP);
        }
    }
}
void Server::send_to_all_udp(const void* data, size_t size) {
    for (auto& clientThread : m_Threads) {
        for (auto& client : clientThread->m_Clients) {
            const auto clientIP = client.second->m_IP;
            const auto clientID = client.second->m_ID;
            auto status = m_UdpSocket->send(m_Port + 1 + clientID, data, size, clientIP);
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
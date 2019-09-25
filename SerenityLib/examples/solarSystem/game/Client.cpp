#include "Client.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "Helper.h"
#include "ResourceManifest.h"
#include "gui/Button.h"
#include "gui/Text.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "gui/specifics/ServerLobbyShipSelectorWindow.h"
#include "gui/specifics/ServerHostingMapSelectorWindow.h"

#include "map/Map.h"
#include "map/Anchor.h"
#include "GameSkybox.h"
#include "Ship.h"
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include "ships/shipSystems/ShipSystemWeapons.h"
#include "weapons/Weapons.h"

#include <iostream>

using namespace std;
using namespace Engine;

const double DISTANCE_CHECK_NEAREST_ANCHOR  = 1000000.0 * 1000000.0;
const double DISTANCE_CHECK_NEAREST_OTHER_PLAYER = 100000.0  * 100000.0;

struct ShipSelectorButtonOnClick final {void operator()(Button* button) const {
    ServerLobbyShipSelectorWindow& window = *static_cast<ServerLobbyShipSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.5f, 0.5f, 0.5f, 0.0f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);

    auto& shipname = button->text();
    auto& handles = ResourceManifest::Ships.at(shipname);
    window.m_ChosenShipName = shipname;
    EntityWrapper& entity = *static_cast<EntityWrapper*>(window.getUserPointer());
    ComponentModel& model = *entity.entity().getComponent<ComponentModel>();
    model.setModelMesh(handles.get<0>(),0);
    model.setModelMaterial(handles.get<1>(), 0);
    model.show();

    auto& camera = const_cast<Camera&>(window.getShipDisplay().getCamera());
    camera.entity().getComponent<ComponentLogic2>()->call(-0.0001);
}};

Client::Client(Core& core, const ushort& server_port, const string& server_ipAddress, const uint& id) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(server_port,          server_ipAddress);
    m_UdpSocket = new Networking::SocketUDP(server_port + 1 + id, server_ipAddress);
    internalInit(server_port, server_ipAddress);
}

Client::~Client() {
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
    SAFE_DELETE(m_UdpSocket);
}
void Client::internalInit(const ushort& server_port, const string& server_ipAddress) {
    m_UdpSocket->setBlocking(false);
    m_UdpSocket->bind();
    m_Port = server_port;
    m_ServerIP = server_ipAddress;
    m_username = "";
    m_mapname = "";
    m_Validated = false;
    m_PingTime = 0.0;
    m_InitialConnectionThread = nullptr;
    m_IsCurrentlyConnecting = false;
}
void Client::setClientID(const uint id) {
    SAFE_DELETE(m_UdpSocket);
    m_UdpSocket = new Networking::SocketUDP(m_Port + 1 + id, m_ServerIP);
    m_UdpSocket->setBlocking(false);
    m_UdpSocket->bind();
}

void Client::changeConnectionDestination(const ushort& port, const string& ipAddress) {
    m_IsCurrentlyConnecting = false;
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
}
const sf::Socket::Status Client::connect(const ushort& timeout) {
    if (m_TcpSocket->isBlocking()) {
        auto lambda = [&](Client* client, const ushort timeout) {
            client->m_IsCurrentlyConnecting = true;
            m_Core.m_HUD->setNormalText("Connecting...", static_cast<float>(timeout) + 2.2f);
            const auto status = client->m_TcpSocket->connect(timeout);
            if (status == sf::Socket::Status::Done) {
                m_Core.m_HUD->setGoodText("Connected!", 2);
                client->m_TcpSocket->setBlocking(false);
                m_Core.requestValidation(m_username);
            }else if (status == sf::Socket::Status::Error) {
                m_Core.m_HUD->setErrorText("Connection to the server failed",20);
            }else if (status == sf::Socket::Status::Disconnected) {
                m_Core.m_HUD->setErrorText("Disconnected from the server",20);
            }
            client->m_IsCurrentlyConnecting = false;
            return status;
        };
        SAFE_DELETE_FUTURE(m_InitialConnectionThread);
        m_InitialConnectionThread = new std::future<sf::Socket::Status>(std::move(std::async(std::launch::async, lambda, this, timeout)));
    }else{
        const auto status = m_TcpSocket->connect(timeout);
        if (status == sf::Socket::Status::Done) {
            m_Core.requestValidation(m_username);
        }
        return status;
    }
    return sf::Socket::Status::Error;
}
void Client::disconnect() {
    m_TcpSocket->disconnect();
    m_UdpSocket->unbind();
}
const sf::Socket::Status Client::send(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_TcpSocket->send(sf_packet);
    return status;
}
const sf::Socket::Status Client::send(sf::Packet& packet) {
    const auto status = m_TcpSocket->send(packet);
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size) {
    const auto status = m_TcpSocket->send(data, size);
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size, size_t& sent) {
    const auto status = m_TcpSocket->send(data, size, sent);
    return status;
}
const sf::Socket::Status Client::receive(sf::Packet& packet) {
    const auto status = m_TcpSocket->receive(packet);
    return status;
}
const sf::Socket::Status Client::receive(void* data, size_t size, size_t& received) {
    const auto status = m_TcpSocket->receive(data,size,received);
    return status;
}
const sf::Socket::Status Client::send_udp(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_UdpSocket->send(m_Port, sf_packet, m_ServerIP);
    return status;
}
const sf::Socket::Status Client::send_udp(sf::Packet& packet) {
    const auto status = m_UdpSocket->send(m_Port, packet, m_ServerIP);
    return status;
}
const sf::Socket::Status Client::send_udp(const void* data, size_t size) {
    const auto status = m_UdpSocket->send(m_Port, data, size, m_ServerIP);
    return status;
}
const sf::Socket::Status Client::receive_udp(sf::Packet& packet) {
    const auto status = m_UdpSocket->receive(packet);
    return status;
}
const sf::Socket::Status Client::receive_udp(void* data, size_t size, size_t& received) {
    const auto status = m_UdpSocket->receive(data, size, received);
    return status;
}

const string& Client::username() const {
    return m_username;
}

void Client::update(Client* _client, const double& dt) {
    if (!_client) 
        return;
    auto& client = *_client;
    client.m_PingTime += dt;

    if (client.m_PingTime > 3.0 && client.m_Core.gameState() != GameState::Game) {
        //hacky way of not d/cing outside the game
        Packet pOut;
        pOut.PacketType = PacketType::Client_To_Server_Periodic_Ping;
        client.send(pOut);
        client.m_PingTime = 0.0;
    }else if (client.m_PingTime > PHYSICS_PACKET_TIMER_LIMIT && client.m_Core.gameState() == GameState::Game) {
        //keep pinging the server, sending your ship physics info
        auto& map = *static_cast<Map*>(Resources::getScene(client.m_mapname));
        auto& playerShip = *map.getPlayer();

        Anchor* finalAnchor = map.getRootAnchor();
        const auto& list = map.getClosestAnchor();
        for (auto& closest : list) {
            finalAnchor = finalAnchor->getChildren().at(closest);
        }
        PacketPhysicsUpdate p(playerShip, map, finalAnchor, list, client.m_username);
        p.PacketType = PacketType::Client_To_Server_Ship_Physics_Update;
        client.send_udp(p);

        auto playerPos = playerShip.getPosition();
        auto nearestAnchorPos = finalAnchor->getPosition();
        double distFromMeToNearestAnchor = static_cast<double>(glm::distance2(nearestAnchorPos, playerPos));
            
        if (distFromMeToNearestAnchor > DISTANCE_CHECK_NEAREST_ANCHOR) {
            for (auto& otherShips : map.getShips()) {
                if (otherShips.first != playerShip.getName()) {
                    auto otherPlayerPos = otherShips.second->getPosition();
                    double distFromMeToOtherPlayer = static_cast<double>(glm::distance2(otherPlayerPos, playerPos));
                    const auto calc = (distFromMeToNearestAnchor - DISTANCE_CHECK_NEAREST_ANCHOR) * 0.5f;
                    if (distFromMeToOtherPlayer < glm::max(calc, DISTANCE_CHECK_NEAREST_OTHER_PLAYER)) {
                        const glm::vec3 midpoint = Math::midpoint(otherPlayerPos, playerPos);

                        PacketMessage pOut;
                        pOut.PacketType = PacketType::Client_To_Server_Request_Anchor_Creation;
                        pOut.r = midpoint.x - nearestAnchorPos.x;
                        pOut.g = midpoint.y - nearestAnchorPos.y;
                        pOut.b = midpoint.z - nearestAnchorPos.z;
                        pOut.data = "";
                        pOut.data += std::to_string(list.size());
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
        client.m_PingTime = 0.0;
    }
    client.onReceiveUDP();
    client.onReceive();
}
void Client::onReceiveUDP() {
    sf::Packet sf_packet_udp;
    const auto& status_udp = receive_udp(sf_packet_udp);
    if (status_udp == sf::Socket::Status::Done) {
        Packet* basePacket = Packet::getPacket(sf_packet_udp);
        if (basePacket && basePacket->validate(sf_packet_udp)) {
            // Data extracted successfully...
            HUD& hud = *m_Core.m_HUD;
            switch (basePacket->PacketType) {
                case PacketType::Server_To_Client_Ship_Physics_Update: {
                    if (m_Core.gameState() == GameState::Game) { //TODO: figure out a way for the server to only send phyiscs updates to clients in the map
                        PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(basePacket);
                        auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));

                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        auto& playername = info[1];
                        auto& shipclass = info[0];
                        auto& ships = map.getShips();

                        Ship* ship = nullptr;
                        if (ships.size() == 0 || !ships.count(playername)) {
                            auto handles = ResourceManifest::Ships[shipclass];
                            auto spawnPosition = map.getSpawnAnchor()->getPosition();
                            auto x = Helper::GetRandomFloatFromTo(-400, 400);
                            auto y = Helper::GetRandomFloatFromTo(-400, 400);
                            auto z = Helper::GetRandomFloatFromTo(-400, 400);
                            auto randOffsetForSafety = glm::vec3(x, y, z);
                            ship = map.createShip(*this, shipclass, playername, false, spawnPosition + randOffsetForSafety);
                        }else{
                            ship = ships.at(playername);
                        }
                        ship->updatePhysicsFromPacket(pI, map, info);
                    }
                    break;
                }default: {
                    break;
                }
            }
        }
    }
}
void Client::onReceive() {
    sf::Packet sf_packet;
    const auto& status     = receive(sf_packet);
    if (status == sf::Socket::Status::Done) {
        Packet* basePacket = Packet::getPacket(sf_packet);
        if (basePacket && basePacket->validate(sf_packet)) {
            // Data extracted successfully...
            HUD& hud = *m_Core.m_HUD;
            switch (basePacket->PacketType) {
                case PacketType::Server_To_Client_Projectile_Cannon_Impact: {
                    PacketProjectileImpact& pI = *static_cast<PacketProjectileImpact*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    auto* targetShip = map.getShips().at(list[0]);
                    if(targetShip)
                        targetShip->updateProjectileImpact(pI);
                    break;
                }case PacketType::Server_To_Client_Projectile_Torpedo_Impact: {
                    PacketProjectileImpact& pI = *static_cast<PacketProjectileImpact*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
                    auto* targetShip = map.getShips().at(list[0]);
                    if(targetShip)
                        targetShip->updateProjectileImpact(pI);
                    break;
                }case PacketType::Server_To_Client_Client_Left_Map: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto& ships = map.getShips();
                    if (map.hasShip(pI.name)) {
                        auto* myShip = map.getPlayer();
                        auto* myTarget = myShip->getTarget();
                        if (myTarget && myTarget->getComponent<ComponentName>()->name() == pI.name) {
                            myShip->setTarget(nullptr, true); //detarget him
                        }
                        auto* removedShip = ships.at(pI.name);
                        removedShip->destroy();
                        ships.erase(pI.name);
                    }
                    break;
                }case PacketType::Server_To_Client_Client_Fired_Cannons: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto& ships = map.getShips();
                    if (map.hasShip(pI.name)) {
                        auto* ship = ships.at(pI.name);
                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        for (uint i = 0; i < info.size() / 2; ++i) {
                            auto& cannon = ship->getPrimaryWeaponCannon(stoi(info[i * 2]));
                            const auto projectile_index = stoi(info[(i * 2) + 1]);
                            const bool success = cannon.forceFire(projectile_index);
                        }
                    }
                    break;
                }case PacketType::Server_To_Client_Client_Fired_Beams: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto& ships = map.getShips();
                    if (map.hasShip(pI.name)) {
                        auto* ship = ships.at(pI.name);
                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        for (uint i = 0; i < info.size() / 2; ++i) {
                            auto& beam = ship->getPrimaryWeaponBeam(stoi(info[i * 2]));
                            const auto projectile_index = stoi(info[(i * 2) + 1]);
                            beam.fire(0.0f);
                        }
                    }
                    break;
                }case PacketType::Server_To_Client_Client_Fired_Torpedos: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto& ships = map.getShips();
                    if (map.hasShip(pI.name)) {
                        auto* ship = ships.at(pI.name);
                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        for (uint i = 0; i < info.size() / 2; ++i) {
                            auto& torpedo = ship->getSecondaryWeaponTorpedo(stoi(info[i * 2]));
                            const auto projectile_index = stoi(info[(i * 2) + 1]);
                            const bool success = torpedo.forceFire(projectile_index);
                        }
                    }
                    break;
                }case PacketType::Server_To_Client_Client_Changed_Target: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto& ships = map.getShips();
                    if (map.hasShip(pI.name)) {
                        ships[pI.name]->setTarget(pI.data, false);
                    }
                    break;
                }case PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto& anchorPos = map.getSpawnAnchor()->getPosition();
                    map.internalCreateDeepspaceAnchor(pI.r + anchorPos.x, pI.g + anchorPos.y, pI.b + anchorPos.z,pI.data);
                    //std::cout << "creating " << pI.data << " Initial" << std::endl;
                    break;
                }case PacketType::Server_To_Client_Anchor_Creation: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto info = Helper::SeparateStringByCharacter(pI.data, ',');

                    const unsigned int& size = stoi(info[0]);
                    Anchor* closest = map.getRootAnchor();
                    for (unsigned int i = 1; i < 1 + size; ++i) {
                        closest = closest->getChildren().at(info[i]);
                    }
                    auto anchorPos = closest->getPosition();
                    const float x = pI.r + anchorPos.x;
                    const float y = pI.g + anchorPos.y;
                    const float z = pI.b + anchorPos.z;
                    map.internalCreateDeepspaceAnchor(x, y, z);
                    //std::cout << "creating deep space anchor" << std::endl;
                    break;
                }case PacketType::Server_To_Client_Ship_Health_Update:{
                    PacketHealthUpdate& pI = *static_cast<PacketHealthUpdate*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                    auto& playername = info[1];
                    auto& shipclass = info[0];
                    auto& ships = map.getShips();
                    if (map.hasShip(playername)) {
                        Ship& ship = *ships.at(playername);
                        ship.updateHealthFromPacket(pI);
                    }
                    break;
                }case PacketType::Server_To_Client_Ship_Cloak_Update: {
                    PacketCloakUpdate& pI = *static_cast<PacketCloakUpdate*>(basePacket);
                    auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));
                    auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                    auto& playername = info[1];
                    auto& shipclass = info[0];
                    auto& ships = map.getShips();

                    if (map.hasShip(playername)) {
                        Ship& ship = *ships.at(playername);
                        ship.updateCloakFromPacket(pI);
                    }
                    break;
                }case PacketType::Server_To_Client_Ship_Physics_Update: {
                    if (m_Core.gameState() == GameState::Game) { //TODO: figure out a way for the server to only send phyiscs updates to clients in the map
                        PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(basePacket);
                        auto& map = *static_cast<Map*>(Resources::getScene(m_mapname));

                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        auto& playername = info[1];
                        auto& shipclass = info[0];
                        auto& ships = map.getShips();

                        Ship* ship = nullptr;
                        if (ships.size() == 0 || !ships.count(playername)) {
                            auto handles = ResourceManifest::Ships[shipclass];
                            auto spawnPosition = map.getSpawnAnchor()->getPosition();
                            auto x = Helper::GetRandomFloatFromTo(-400, 400);
                            auto y = Helper::GetRandomFloatFromTo(-400, 400);
                            auto z = Helper::GetRandomFloatFromTo(-400, 400);
                            auto randOffsetForSafety = glm::vec3(x, y, z);
                            ship = map.createShip(*this, shipclass, playername, false, spawnPosition + randOffsetForSafety);
                        }else{
                            ship = ships.at(playername);
                        }
                        ship->updatePhysicsFromPacket(pI, map, info);
                    }
                    break;
                }case PacketType::Server_To_Client_New_Client_Entered_Map: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    
                    auto info = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map

                    Map& map = *static_cast<Map*>(Resources::getScene(info[1]));

                    auto spawn = map.getSpawnAnchor()->getPosition();
                    Ship* ship = map.createShip(*this, info[0], pI.name, false, glm::vec3(pI.r + spawn.x, pI.g + spawn.y, pI.b + spawn.z));
                    if (ship) { //if the ship was successfully added
                        //send the new guy several of our statuses
                        auto player = map.getPlayer();
                        if (player) {
                            //cloak status
                            PacketCloakUpdate pOut1(*player);
                            pOut1.PacketType = PacketType::Client_To_Server_Ship_Cloak_Update;
                            pOut1.data += ("," + pI.name);
                            send(pOut1);

                            //target status
                            player->setTarget(player->getTarget(), true); //sends target packet info to the new guy

                            //health status
                            PacketHealthUpdate pOut2(*player);
                            pOut2.PacketType = PacketType::Client_To_Server_Ship_Health_Update;
                            pOut1.data += ("," + pI.name);
                            send(pOut2);
                        }
                    }
                    break;
                }case PacketType::Server_To_Client_Approve_Map_Entry: {
                    //ok the server let me in, let me tell the server i successfully went in
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);

                    auto info = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map

                    hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
                    m_Core.enterMap(info[1], info[0], pI.name, pI.r, pI.g, pI.b);
                    hud.m_Next->setText("Next");
                    hud.m_GameState = GameState::Game;//ok, ive entered the map
                    Map& map = *static_cast<Map*>(Resources::getScene(info[1]));

                    auto dist = Helper::GetRandomFloatFromTo(200, 250);
                    auto sin = Helper::GetRandomFloatFromTo(0, 2 * 3.14159f);
                    auto cos = Helper::GetRandomFloatFromTo(0, 2 * 3.14159f);

                    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                    Math::rotate(orientation, sin, cos, 0.0f);


                    glm::mat4 modelMatrix = glm::mat4(1.0f);
                    auto position = glm::vec3(0, 0, -dist);
                    modelMatrix = glm::mat4_cast(orientation) * glm::translate(position);
                    auto& playerBody = *map.getPlayer()->getComponent<ComponentBody>();
                    auto spawn = map.getSpawnAnchor()->getPosition();
                    playerBody.setPosition(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);

                    Math::alignTo(orientation, playerBody.position() - spawn);
                    playerBody.setRotation(orientation);

                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Client_To_Server_Successfully_Entered_Map;
                    pOut.r = modelMatrix[3][0] - spawn.x;
                    pOut.g = modelMatrix[3][1] - spawn.y;
                    pOut.b = modelMatrix[3][2] - spawn.z;
                    send(pOut);
                    break;
                }case PacketType::Server_To_Client_Reject_Map_Entry: {
                    break;
                }case PacketType::Server_To_Client_Map_Data: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto& mapname = pI.name;
                    m_mapname = mapname;
                    Map* map = static_cast<Map*>(Resources::getScene(mapname));
                    if (!map) {
                        map = new Map(*this, mapname, ResourceManifest::BasePath + "data/Systems/" + mapname + ".txt");
                    }
                    auto& menuScene = *const_cast<Scene*>(Resources::getScene("Menu"));
                    auto* menuSkybox = menuScene.skybox();
                    SAFE_DELETE(menuSkybox);
                    GameSkybox* newMenuSkybox = new GameSkybox(map->skyboxFile(), 0);
                    menuScene.setSkybox(newMenuSkybox);
                    menuScene.setGlobalIllumination(map->getGlobalIllumination());

                    hud.m_ServerLobbyShipSelectorWindow->clear();
                    auto ships = map->allowedShips();
                    for (auto& ship : ships) {
                        auto& handles = ResourceManifest::Ships.at(ship);
                        auto& r = handles.get<2>().r;
                        auto& g = handles.get<2>().g;
                        auto& b = handles.get<2>().b;

                        Button* shipbutton = new Button(*hud.m_Font, 0, 0, 100, 40);
                        shipbutton->setText(ship);
                        shipbutton->setColor(0.5f, 0.5f, 0.5f, 0.0f);
                        shipbutton->setTextColor(r, g, b, 1.0f);
                        shipbutton->setAlignment(Alignment::TopLeft);
                        shipbutton->setWidth(600);
                        shipbutton->setTextAlignment(TextAlignment::Left);
                        shipbutton->setUserPointer(hud.m_ServerLobbyShipSelectorWindow);
                        shipbutton->setOnClickFunctor(ShipSelectorButtonOnClick());
                        hud.m_ServerLobbyShipSelectorWindow->addContent(shipbutton);
                    }
                    hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(true);
                    break;
                }case PacketType::Server_To_Client_Chat_Message: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto message = pI.name + ": " + pI.data;

                    Text* text = new Text(0, 0, *hud.m_Font, message);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text);
                    break;
                
                }case PacketType::Server_To_Client_Client_Joined_Server: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto message = pI.name + ": Has joined the server";


                    Text* text = new Text(0, 0, *hud.m_Font, pI.name);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyConnectedPlayersWindow->addContent(text);

                    Text* text1 = new Text(0, 0, *hud.m_Font, message);
                    text1->setColor(0.8f, 1, 0.2f, 1);
                    text1->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text1);
                    break;
                }case PacketType::Server_To_Client_Client_Left_Server:{
                    PacketMessage& pI = *static_cast<PacketMessage*>(basePacket);
                    auto message = pI.name + ": Has left the server";

                    Text* text = new Text(0, 0, *hud.m_Font, pI.name);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyConnectedPlayersWindow->removeContent(pI.name);

                    Text* text1 = new Text(0, 0, *hud.m_Font, message);
                    text1->setColor(0.907f, 0.341f, 0.341f, 1.0f);
                    text1->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text1);
                    break;
                }case PacketType::Server_To_Client_Accept_Connection: {
                    m_Validated = true;
                    if (m_Core.m_GameState != GameState::Host_Server_Lobby_And_Ship && m_Core.m_GameState == GameState::Host_Server_Port_And_Name_And_Map) {
                        m_Core.m_GameState = GameState::Host_Server_Lobby_And_Ship;
                        hud.m_Next->setText("Enter Game");
                    }else if (m_Core.m_GameState != GameState::Join_Server_Server_Lobby && m_Core.m_GameState == GameState::Join_Server_Port_And_Name_And_IP) {
                        m_Core.m_GameState = GameState::Join_Server_Server_Lobby;
                        hud.m_Next->setText("Enter Game");
                    }
                    hud.m_ServerLobbyConnectedPlayersWindow->clear();
                    hud.m_ServerLobbyChatWindow->clear();

                    auto list = Helper::SeparateStringByCharacter(basePacket->data, ',');
                    auto client_id = list.back();
                    list.pop_back();
                    //list is a vector of connected players
                    for (auto& _name : list) {
                        if (!_name.empty()) { //trailing "," in data can lead to an empty string added into the list
                            Text* text = new Text(0, 0, *hud.m_Font, _name);
                            text->setColor(1, 1, 0, 1);
                            text->setTextScale(0.62f, 0.62f);
                            hud.m_ServerLobbyConnectedPlayersWindow->addContent(text);
                        }
                    }
                    setClientID(std::stoi(client_id));
                    break;
                }case PacketType::Server_To_Client_Reject_Connection: {
                    m_Validated = false;
                    hud.setErrorText("Someone has already chosen that name");
                    break;
                }case PacketType::Server_Shutdown: {
                    m_Validated = false;
                    m_Core.shutdownClient(true);
                    hud.setErrorText("Disconnected from the server",600);
                    m_Core.m_GameState = GameState::Main_Menu;
                    hud.m_Next->setText("Next");

                    hud.m_ServerLobbyChatWindow->clear();
                    hud.m_ServerLobbyConnectedPlayersWindow->clear();
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(basePacket);
    }
}
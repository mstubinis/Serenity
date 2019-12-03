#include "Client.h"
#include "../packets/Packets.h"
#include "../../Core.h"
#include "../../Menu.h"
#include "../../Helper.h"
#include "../../ResourceManifest.h"
#include "../../gui/Button.h"
#include "../../gui/Text.h"
#include "../../gui/specifics/ServerLobbyChatWindow.h"
#include "../../gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "../../gui/specifics/ServerLobbyShipSelectorWindow.h"
#include "../../gui/specifics/ServerHostingMapSelectorWindow.h"

#include "../../map/Map.h"
#include "../../map/Anchor.h"
#include "../../GameSkybox.h"
#include "../../Ship.h"

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

#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"
#include "../../ships/shipSystems/ShipSystemHull.h"
#include "../../weapons/Weapons.h"
#include "../../ships/Ships.h"

#include "../../teams/Team.h"
#include "../../modes/GameplayMode.h"

#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>

#include "../../particles/Fire.h"
#include "../../particles/Sparks.h"
#include <core/engine/physics/Collision.h>

#include <iostream>

using namespace std;
using namespace Engine;

struct ShipSelectorButtonOnClick final {void operator()(Button* button) const {
    ServerLobbyShipSelectorWindow& window = *static_cast<ServerLobbyShipSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.5f, 0.5f, 0.5f, 0.0f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);

    auto& shipClass = button->text();
    auto& shipData = Ships::Database.at(shipClass);
    window.m_ChosenShipName = shipClass;
    auto& entity = *static_cast<EntityWrapper*>(window.getUserPointer());
    auto& model = *entity.entity().getComponent<ComponentModel>();
    model.setModelMesh(shipData.MeshHandles[0], 0);
    model.setModelMaterial(shipData.MaterialHandles[0], 0);
    model.show();

    auto& camera = const_cast<Camera&>(window.getShipDisplay().getCamera());
    camera.entity().getComponent<ComponentLogic2>()->call(-0.0001);
}};

Client::Client(Team* team, Core& core, const unsigned short& server_port, const string& server_ipAddress, const unsigned int& id) : m_Core(core), m_MapSpecificData(*this){
    m_MapSpecificData.m_Team = team;
    m_TcpSocket = new Networking::SocketTCP(server_port,          server_ipAddress);
    m_UdpSocket = new Networking::SocketUDP(server_port + 1 + id, server_ipAddress);
    internal_init(server_port, server_ipAddress);
}

Client::~Client() {
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
    SAFE_DELETE(m_UdpSocket);
}
void Client::internal_init(const unsigned short& server_port, const string& server_ipAddress) {
    m_UdpSocket->setBlocking(false);
    m_UdpSocket->bind();
    m_Port                    = server_port;
    m_ServerIP                = server_ipAddress;
    m_Username                = "";
    m_InitialConnectionThread = nullptr;
}
void Client::setClientID(const unsigned int id) {
    SAFE_DELETE(m_UdpSocket);
    m_UdpSocket = new Networking::SocketUDP(m_Port + 1 + id, m_ServerIP);
    m_UdpSocket->setBlocking(false);
    m_UdpSocket->bind();
}
const bool Client::isReadyForConnection() const {
    if (m_InitialConnectionThread) {
        if (!m_InitialConnectionThread->_Is_ready()) {
            return false;
        }
    }
    return true;
}
void Client::changeConnectionDestination(const unsigned short& port, const string& ipAddress) {
    if (!isReadyForConnection())
        return;
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
}
void Client::connect(const unsigned short& timeout) {
    auto lambda_connect = [&](const unsigned short timeout) {
        m_Core.m_Menu->setNormalText("Connecting...", static_cast<float>(timeout) + 3.2f);
        const auto status = m_TcpSocket->connect(timeout);
        if (status == sf::Socket::Status::Done) {
            m_Core.m_Menu->setGoodText("Connected!", 2);
            m_TcpSocket->setBlocking(false);
            m_Core.requestValidation(m_Username);
        }else if (status == sf::Socket::Status::Error) {
            m_Core.m_Menu->setErrorText("Connection to the server failed", 20);
        }else if (status == sf::Socket::Status::Disconnected) {
            m_Core.m_Menu->setErrorText("Disconnected from the server", 20);
        }else {

        }
        return status;
    };
    if (!isReadyForConnection())
        return;
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    m_InitialConnectionThread = new std::future<sf::Socket::Status>(std::move(std::async(std::launch::async, lambda_connect, timeout)));
}
GameplayMode* Client::getGameplayMode() {
    return m_MapSpecificData.m_GameplayMode;
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
    return m_Username;
}

void Client::update(Client* _client, const double& dt) {
    if (!_client) 
        return;
    auto& client = *_client;

    if (client.m_Core.gameState() == GameState::Game) {
        client.m_MapSpecificData.update(dt);
    }
    client.onReceiveUDP();
    client.onReceiveTCP();
}

void Client::on_receive_physics_update(Packet& basePacket, Map& map) {
    if (m_Core.gameState() == GameState::Game) { //TODO: figure out a way for the server to only send phyiscs updates to clients in the map
        PacketPhysicsUpdate& pI     = static_cast<PacketPhysicsUpdate&>(basePacket);
        TeamNumber::Enum teamNumber = static_cast<TeamNumber::Enum>(pI.team_number);
        AIType::Type aiType         = static_cast<AIType::Type>(pI.ai_type);
        auto& ships                 = map.getShips();
        Ship* new_ship = nullptr;


        if (ships.size() == 0 || !ships.count(pI.ship_map_key)) {
            auto spawnPosition = map.getSpawnAnchor()->getPosition();
            auto x = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
            auto y = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
            auto z = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
            auto randOffsetForSafety = glm_vec3(x, y, z);

            Ship* player_you = map.getPlayer();

            AIType::Type final_ai;
            if (aiType == AIType::Player_You && player_you)
                final_ai = AIType::Player_Other;
            //TODO: create proper AI for npc's
            new_ship = map.createShip(final_ai, *m_MapSpecificData.m_GameplayMode->getTeams().at(teamNumber), *this, pI.ship_class, pI.player_username, spawnPosition + randOffsetForSafety);
            if (new_ship) {
                //hey a new ship entered my map, i want info about it!
                PacketMessage pOut;
                pOut.PacketType = PacketType::Client_To_Server_Request_Ship_Current_Info;
                pOut.name       = player_you->getMapKey();
                pOut.data       = new_ship->getMapKey();
                pOut.data      += "," + pI.player_username;
                send(pOut);
            }
        }else{
            new_ship = ships.at(pI.ship_map_key);
        }
        new_ship->updatePhysicsFromPacket(pI, map);
    }
}

void Client::on_receive_ship_notified_of_respawn(Packet& basePacket, Map& map) {
    PacketShipRespawned& pI = static_cast<PacketShipRespawned&>(basePacket);
    auto respawnPosition = glm_vec3(static_cast<decimal>(pI.x), static_cast<decimal>(pI.y), static_cast<decimal>(pI.z));
    if (map.hasShip(pI.respawned_ship_map_key)) {
        auto& ship = *map.getShips().at(pI.respawned_ship_map_key);
        ship.respawn(respawnPosition, pI.nearest_spawn_anchor_name, map);
    }
}
void Client::on_receive_ship_was_just_destroyed(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
}
void Client::on_receive_ship_notified_of_impending_respawn(Packet& basePacket, Map& map) {
    PacketShipRespawnNotification& pI = static_cast<PacketShipRespawnNotification&>(basePacket);
    auto* my_Ship_ptr = map.getPlayer();
    if (my_Ship_ptr) {
        auto& my_ship = *my_Ship_ptr;
        my_ship.setState(ShipState::JustFlaggedToRespawn);
        my_ship.m_RespawnTimerMax = pI.timer;
    }
}
void Client::on_receive_client_wants_my_ship_info(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
    auto* my_Ship_ptr = map.getPlayer();
    //auto* source_ship = map.getShips().at(pI.name);
    //const auto& source_ship_mapkey = source_ship->getMapKey();

    if (my_Ship_ptr) {
        auto& my_ship = *my_Ship_ptr;
        //TODO: add ALOT more here

        //health status
        PacketHealthUpdate pOut2(my_ship);
        pOut2.PacketType = PacketType::Client_To_Server_Ship_Health_Update;
        send(pOut2);

        //cloak status
        PacketCloakUpdate pOut1(my_ship);
        pOut1.PacketType = PacketType::Client_To_Server_Ship_Cloak_Update;
        send(pOut1);

        //target status
        my_ship.setTarget(my_ship.getTarget(), true); //sends target packet info to the new guy

        //anti cloak scan status
        auto* sensors = static_cast<ShipSystemSensors*>(my_ship.getShipSystem(ShipSystemType::Sensors));
        if (sensors) {
            sensors->sendAntiCloakScanStatusPacket();
        }
    }
}
void Client::on_receive_collision_event(Packet& basePacket, Map& map) {
    PacketCollisionEvent& pI = static_cast<PacketCollisionEvent&>(basePacket);
    Ship* ship1 = map.getShips().at(pI.owner_key);
    Ship* ship2 = map.getShips().at(pI.other_key);
    if (ship1 && ship2) {
        auto* hull1 = static_cast<ShipSystemHull*>(ship1->getShipSystem(ShipSystemType::Hull));
        auto* hull2 = static_cast<ShipSystemHull*>(ship2->getShipSystem(ShipSystemType::Hull));
        hull1->receiveCollisionDamage(pI.other_key, pI.damage1);
        hull2->receiveCollisionDamage(pI.owner_key, pI.damage2);

        auto& body1 = *ship1->getComponent<ComponentBody>();
        auto& body2 = *ship2->getComponent<ComponentBody>();

        
        glm::vec3 av1, av2, lv1, lv2;
        Math::Float32From16(&av1.x, pI.ax1);
        Math::Float32From16(&av1.y, pI.ay1);
        Math::Float32From16(&av1.z, pI.az1);

        Math::Float32From16(&av2.x, pI.ax2);
        Math::Float32From16(&av2.y, pI.ay2);
        Math::Float32From16(&av2.z, pI.az2);

        Math::Float32From16(&lv1.x, pI.lx1);
        Math::Float32From16(&lv1.y, pI.ly1);
        Math::Float32From16(&lv1.z, pI.lz1);

        Math::Float32From16(&lv2.x, pI.lx2);
        Math::Float32From16(&lv2.y, pI.ly2);
        Math::Float32From16(&lv2.z, pI.lz2);

        if (body1.getCollision()->getType() != CollisionType::TriangleShapeStatic) {
            body1.setLinearVelocity(lv1);
            body1.setAngularVelocity(av1);
        }
        if (body2.getCollision()->getType() != CollisionType::TriangleShapeStatic) {
            body2.setLinearVelocity(lv2);
            body2.setAngularVelocity(av2);
        }
    }
}

void Client::on_receive_anti_cloak_status(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    if (map.hasShip(pI.name)) {
        auto& ships = map.getShips();
        Ship& ship = *ships.at(pI.name);
        ship.updateAntiCloakScanFromPacket(pI);
    }
}

void Client::on_receive_server_game_mode(Packet& basePacket) {
    PacketGameplayModeInfo& pI = static_cast<PacketGameplayModeInfo&>(basePacket);
    m_MapSpecificData.m_GameplayMode->deserialize(pI);
}
void Client::on_receive_cannon_impact(Packet& basePacket, Map& map) {
    PacketProjectileImpact& pI = static_cast<PacketProjectileImpact&>(basePacket);
    auto* impacted_ship = map.getShips().at(pI.impacted_ship_map_key);
    if (impacted_ship) {
        impacted_ship->updateProjectileImpact(pI);
    }
}
void Client::on_receive_torpedo_impact(Packet& basePacket, Map& map) {
    PacketProjectileImpact& pI = static_cast<PacketProjectileImpact&>(basePacket);
    auto* impacted_ship = map.getShips().at(pI.impacted_ship_map_key);
    if (impacted_ship) {
        impacted_ship->updateProjectileImpact(pI);
    }
}
void Client::on_receive_client_left_map(Packet& basePacket, Map& map) {
    PacketMessage& pI        = static_cast<PacketMessage&>(basePacket);
    auto& ships              = map.getShips();
    auto& destroyed_ship_key = pI.name;
    if (map.hasShip(destroyed_ship_key)) {
        auto& my_ship        = *map.getPlayer();
        auto* my_target      = my_ship.getTarget();
        auto* removed_entity = ships.at(destroyed_ship_key);
        Ship* removed_ship   = dynamic_cast<Ship*>(removed_entity);
        Ship* my_target_ship = dynamic_cast<Ship*>(my_target);

        if (removed_ship && my_target_ship && removed_ship->getMapKey() == my_target_ship->getMapKey()) {
            my_ship.setTarget(nullptr, true); //detarget him
        }else{
            if (my_target && my_target->getComponent<ComponentName>()->name() == destroyed_ship_key) {
                my_ship.setTarget(nullptr, true); //detarget him
            }
        }
        removed_entity->destroy();
        ships.erase(destroyed_ship_key);
    }
}
void Client::on_receive_client_fired_cannons(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    auto& ships = map.getShips();
    if (map.hasShip(pI.name)) {
        Ship* ship_that_fired             = ships.at(pI.name);
        const auto chosen_target_position = glm::vec3(pI.r, pI.g, pI.b);
        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
        for (size_t i = 0; i < info.size() / 3; ++i) {
            auto& cannon                  = ship_that_fired->getPrimaryWeaponCannon(stoi(info[i * 3]));
            const auto projectile_index   = stoi(info[(i * 3) + 1]);
            const auto target_name        = info[(i * 3) + 2];
            auto* ship_that_fired_target  = map.getEntityFromName(target_name);
            const bool success            = cannon.forceFire(ship_that_fired_target, projectile_index, chosen_target_position);
        }
    }
}
void Client::on_receive_client_fired_beams(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    auto& ships = map.getShips();
    if (map.hasShip(pI.name)) {
        Ship* ship_that_fired  = ships.at(pI.name);
        auto info              = Helper::SeparateStringByCharacter(pI.data, ',');
        for (size_t i = 0; i < info.size() / 2; ++i) {
            auto& beam                    = ship_that_fired->getPrimaryWeaponBeam(stoi(info[i * 2]));
            const auto target_name        = info[(i * 2) + 1];

            beam.setTarget(map.getEntityFromName(target_name));
            auto* mytarget                = beam.getTarget();
            glm::vec3 chosen_impact_pt    = glm::vec3(pI.r, pI.g, pI.b);
            beam.fire(0.0f, chosen_impact_pt);
        }
    }
}
void Client::on_receive_client_fired_torpedos(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    if (map.hasShip(pI.name)) {
        auto& ships = map.getShips();
        Ship* ship_that_fired             = ships.at(pI.name);
        auto info                         = Helper::SeparateStringByCharacter(pI.data, ',');
        const auto chosen_target_position = glm::vec3(pI.r, pI.g, pI.b);
        for (size_t i = 0; i < info.size() / 3; ++i) {
            auto& torpedo                 = ship_that_fired->getSecondaryWeaponTorpedo(stoi(info[i * 3]));
            const auto projectile_index   = stoi(info[(i * 3) + 1]);
            const auto target_name        = info[(i * 3) + 2];
            auto* ship_that_fired_target  = map.getEntityFromName(target_name);
            const bool success            = torpedo.forceFire(ship_that_fired_target, projectile_index, chosen_target_position);
        }
    }
}
void Client::on_receive_target_changed(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    auto& ship_that_changed_target_key = pI.name;
    if (map.hasShip(ship_that_changed_target_key)) {
        Ship* ship_that_changed_target = map.getShips().at(ship_that_changed_target_key);
        ship_that_changed_target->setTarget(pI.data, false);
    }
}
void Client::on_receive_create_deep_space_anchor_initial(Packet& basePacket, Map& map) {
    PacketMessage& pI          = static_cast<PacketMessage&>(basePacket);
    const auto anchor_position = map.getSpawnAnchor()->getPosition();
    const auto x               = pI.r + anchor_position.x;
    const auto y               = pI.g + anchor_position.y;
    const auto z               = pI.b + anchor_position.z;
    map.internalCreateDeepspaceAnchor(x, y, z, pI.data);
}
void Client::on_receive_create_deep_space_anchor(Packet& basePacket, Map& map) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    const auto info   = Helper::SeparateStringByCharacter(pI.data, ',');

    const size_t size = stoi(info[0]);
    Anchor* closest   = map.getRootAnchor();
    for (size_t i = 1; i < 1 + size; ++i) {
        closest = closest->getChildren().at(info[i]);
    }
    const auto anchorPos = closest->getPosition();
    const auto x         = pI.r + anchorPos.x;
    const auto y         = pI.g + anchorPos.y;
    const auto z         = pI.b + anchorPos.z;
    map.internalCreateDeepspaceAnchor(x, y, z);
}
void Client::on_receive_health_update(Packet& basePacket, Map& map) {
    PacketHealthUpdate& pI = static_cast<PacketHealthUpdate&>(basePacket);
    if (map.hasShip(pI.ship_map_key)) {
        Ship& ship_that_changed_health = *map.getShips().at(pI.ship_map_key);
        ship_that_changed_health.updateHealthFromPacket(pI);
    }
}
void Client::on_receive_cloak_update(Packet& basePacket, Map& map) {
    PacketCloakUpdate& pI = static_cast<PacketCloakUpdate&>(basePacket);
    if (map.hasShip(pI.ship_map_key) && map.getPlayer()) {
        Ship& ship_that_changed_cloaking = *map.getShips().at(pI.ship_map_key);
        ship_that_changed_cloaking.updateCloakFromPacket(pI);
    }
}
void Client::on_receive_new_client_entered_map(Packet& basePacket) {
    PacketMessage& pI           = static_cast<PacketMessage&>(basePacket);
    const auto list             = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map,teamNumber
    TeamNumber::Enum teamNumber = static_cast<TeamNumber::Enum>(stoi(list[2]));
    Map& map                    = *static_cast<Map*>(Resources::getScene(list[1]));
    const auto spawn_anchor_pos = map.getSpawnAnchor()->getPosition();
    Ship* ship                  = map.createShip(AIType::Player_Other, *m_MapSpecificData.m_GameplayMode->getTeams().at(teamNumber), *this, list[0], pI.name, glm::vec3(pI.r + spawn_anchor_pos.x, pI.g + spawn_anchor_pos.y, pI.b + spawn_anchor_pos.z));
}
void Client::on_receive_server_approve_map_entry(Packet& basePacket, Menu& menu) {
    //ok the server let me in, let me tell the server i successfully went in
    PacketMessage& pI           = static_cast<PacketMessage&>(basePacket);
    const auto list             = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map,teamNumber
    TeamNumber::Enum teamNumber = static_cast<TeamNumber::Enum>(stoi(list[2]));

    menu.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
    m_Core.enterMap(*m_MapSpecificData.m_GameplayMode->getTeams().at(teamNumber), list[1], list[0], pI.name, pI.r, pI.g, pI.b);
    menu.m_Next->setText("Next");
    menu.m_GameState = GameState::Game;//ok, ive entered the map

    auto dist = Helper::GetRandomFloatFromTo(200, 250);
    auto sin  = Helper::GetRandomFloatFromTo(0, 2 * 3.14159f);
    auto cos  = Helper::GetRandomFloatFromTo(0, 2 * 3.14159f);

    auto orientation = glm_quat(1.0f, 0.0f, 0.0f, 0.0f);
    Math::rotate(orientation, sin, cos, 0.0f);

    auto& map = *m_MapSpecificData.m_Map;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    auto position = glm_vec3(0, 0, -dist);
    modelMatrix = glm::mat4_cast(orientation) * glm::translate(position);
    auto& playerBody = *map.getPlayer()->getComponent<ComponentBody>();
    auto spawn = map.getSpawnAnchor()->getPosition();
    playerBody.setPosition(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);

    Math::alignTo(orientation, playerBody.position() - spawn);
    playerBody.setRotation(orientation);

    //TODO: just testing particles here....
    /*
    glm::vec2 pos;
    for (size_t i = 0; i < 50; ++i) {
        for (size_t j = 0; j < 50; ++j) {
            pos = glm::vec2(i, j);

            ParticleEmitter e(*Fire::m_Properties, map, 0.0);
            e.getComponent<ComponentBody>()->setPosition(playerBody.position() + glm_vec3(pos.x, 0, pos.y));
            map.addParticleEmitter(e);
        }
    }
    ParticleEmitter e(*Sparks::Spray, map, 0.0,nullptr);
    e.getComponent<ComponentBody>()->setPosition(playerBody.position());
    map.addParticleEmitter(e);
    */
    
    PacketMessage pOut(pI);
    pOut.PacketType = PacketType::Client_To_Server_Successfully_Entered_Map;
    pOut.name       = map.getPlayer()->getMapKey();
    pOut.r          = modelMatrix[3][0] - static_cast<float>(spawn.x);
    pOut.g          = modelMatrix[3][1] - static_cast<float>(spawn.y);
    pOut.b          = modelMatrix[3][2] - static_cast<float>(spawn.z);
    send(pOut);
}
void Client::on_receive_map_data(Packet& basePacket, Menu& menu) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    if (!m_MapSpecificData.m_Map) {
        new Map(*m_MapSpecificData.m_GameplayMode, *this, pI.name, ResourceManifest::BasePath + "data/Systems/" + pI.name + ".txt");
        Map* map_ptr_real = static_cast<Map*>(Resources::getScene(pI.name));
        m_MapSpecificData.m_Map = map_ptr_real;
    }
    auto& map = *m_MapSpecificData.m_Map;
    auto& menuScene = *const_cast<Scene*>(Resources::getScene("Menu"));
    auto* menuSkybox = menuScene.skybox();
    SAFE_DELETE(menuSkybox);
    GameSkybox* newMenuSkybox = new GameSkybox(map.skyboxFile(), 0);
    menuScene.setSkybox(newMenuSkybox);
    menuScene.setGlobalIllumination(map.getGlobalIllumination());

    menu.m_ServerLobbyShipSelectorWindow->clear();
    auto allowed_ships = map.allowedShips();
    for (auto& allowed_ship : allowed_ships) {
        auto& textColor = Ships::Database.at(allowed_ship).FactionInformation.ColorText;
        Button* shipbutton = new Button(*menu.m_Font, 0, 0, 100, 40);
        shipbutton->setText(allowed_ship);
        shipbutton->setColor(0.5f, 0.5f, 0.5f, 0.0f);
        shipbutton->setTextColor(textColor.r, textColor.g, textColor.b, 1.0f);
        shipbutton->setAlignment(Alignment::TopLeft);
        shipbutton->setWidth(600);
        shipbutton->setTextAlignment(TextAlignment::Left);
        shipbutton->setUserPointer(menu.m_ServerLobbyShipSelectorWindow);
        shipbutton->setOnClickFunctor(ShipSelectorButtonOnClick());
        menu.m_ServerLobbyShipSelectorWindow->addContent(shipbutton);
    }
    menu.m_ServerLobbyShipSelectorWindow->setShipViewportActive(true);
}
void Client::on_receive_chat_message(Packet& basePacket, Menu& menu) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    const auto message = pI.name + ": " + pI.data;
    Text* text = new Text(0, 0, *menu.m_Font, message);
    text->setColor(1, 1, 0, 1);
    text->setTextScale(0.62f, 0.62f);
    menu.m_ServerLobbyChatWindow->addContent(text);
}
void Client::on_receive_client_just_joined_server_lobby(Packet& basePacket, Menu& menu) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    const auto message = pI.name + ": Has joined the server";

    Text* text = new Text(0, 0, *menu.m_Font, pI.name);
    text->setColor(1, 1, 0, 1);
    text->setTextScale(0.62f, 0.62f);
    menu.m_ServerLobbyConnectedPlayersWindow->addContent(text);

    Text* text1 = new Text(0, 0, *menu.m_Font, message);
    text1->setColor(0.8f, 1, 0.2f, 1);
    text1->setTextScale(0.62f, 0.62f);
    menu.m_ServerLobbyChatWindow->addContent(text1);
}
void Client::on_receive_client_just_left_server(Packet& basePacket, Menu& menu) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    const auto message = pI.name + ": Has left the server";

    Text* text = new Text(0, 0, *menu.m_Font, pI.name);
    text->setColor(1, 1, 0, 1);
    text->setTextScale(0.62f, 0.62f);
    menu.m_ServerLobbyConnectedPlayersWindow->removeContent(pI.name);

    Text* text1 = new Text(0, 0, *menu.m_Font, message);
    text1->setColor(0.907f, 0.341f, 0.341f, 1.0f);
    text1->setTextScale(0.62f, 0.62f);
    menu.m_ServerLobbyChatWindow->addContent(text1);
}
void Client::on_receive_connection_accepted_by_server(Packet& basePacket, Menu& menu) {
    PacketMessage& pI = static_cast<PacketMessage&>(basePacket);
    if (m_Core.m_GameState != GameState::Host_Server_Lobby_And_Ship && m_Core.m_GameState == GameState::Host_Server_Port_And_Name_And_Map) {
        m_Core.m_GameState = GameState::Host_Server_Lobby_And_Ship;
        menu.m_Next->setText("Enter Game");
    }else if (m_Core.m_GameState != GameState::Join_Server_Server_Lobby && m_Core.m_GameState == GameState::Join_Server_Port_And_Name_And_IP) {
        m_Core.m_GameState = GameState::Join_Server_Server_Lobby;
        menu.m_Next->setText("Enter Game");
    }
    menu.m_ServerLobbyConnectedPlayersWindow->clear();
    menu.m_ServerLobbyChatWindow->clear();

    auto list = Helper::SeparateStringByCharacter(pI.data, ',');
    auto client_id = stoi(list.back());
    list.pop_back();
    //list is a vector of connected players
    for (auto& _name : list) {
        if (!_name.empty()) { //trailing "," in data can lead to an empty string added into the list
            Text* text = new Text(0, 0, *menu.m_Font, _name);
            text->setColor(1, 1, 0, 1);
            text->setTextScale(0.62f, 0.62f);
            menu.m_ServerLobbyConnectedPlayersWindow->addContent(text);
        }
    }
    setClientID(client_id);
}
void Client::on_receive_connection_rejected_by_server(Packet& basePacket, Menu& menu) {
    menu.setErrorText("Someone has already chosen that name");
}
void Client::on_receive_server_shutdown(Packet& basePacket, Menu& menu, Map& map) {
    Resources::setCurrentScene("Menu");
    Resources::deleteScene(map);
    m_Core.shutdownClient(true);
    menu.setErrorText("Disconnected from the server", 600);
    m_Core.m_GameState = GameState::Main_Menu;
    menu.m_Next->setText("Next");
    menu.m_ServerLobbyChatWindow->clear();
    menu.m_ServerLobbyConnectedPlayersWindow->clear();
}

void Client::onReceiveUDP() {
    sf::Packet sf_packet;
    const auto status = receive_udp(sf_packet);
    if (status == sf::Socket::Status::Done) {
        Packet* basePacket_Ptr = Packet::getPacket(sf_packet);
        if (basePacket_Ptr && basePacket_Ptr->validate(sf_packet)) {
            auto& basePacket = *basePacket_Ptr;
            Menu& menu = *m_Core.m_Menu;
            Map& map = *m_MapSpecificData.m_Map;
            switch (basePacket.PacketType) {
                case PacketType::Server_To_Client_Ship_Physics_Update: {
                    on_receive_physics_update(basePacket, map);
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(basePacket_Ptr);
    }
}
void Client::onReceiveTCP() {
    sf::Packet sf_packet;
    const auto status = receive(sf_packet);
    if (status == sf::Socket::Status::Done) {
        Packet* basePacket_Ptr = Packet::getPacket(sf_packet);
        if (basePacket_Ptr && basePacket_Ptr->validate(sf_packet)) {
            auto& basePacket = *basePacket_Ptr;
            Menu& menu = *m_Core.m_Menu;
            if (m_MapSpecificData.m_Map) {
                auto& map = *m_MapSpecificData.m_Map;
                switch (basePacket.PacketType) {
                    case PacketType::Server_To_Client_Notify_Ship_Of_Respawn: {
                        on_receive_ship_notified_of_respawn(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Notify_Ship_Of_Impending_Respawn: {
                        on_receive_ship_notified_of_impending_respawn(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Ship_Was_Just_Destroyed: {
                        on_receive_ship_was_just_destroyed(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Request_Ship_Current_Info: {
                        on_receive_client_wants_my_ship_info(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Collision_Event: {
                        on_receive_collision_event(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Anti_Cloak_Status: {
                        on_receive_anti_cloak_status(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Projectile_Cannon_Impact: {
                        on_receive_cannon_impact(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Projectile_Torpedo_Impact: {
                        on_receive_torpedo_impact(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Client_Left_Map: {
                        on_receive_client_left_map(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Client_Fired_Cannons: {
                        on_receive_client_fired_cannons(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Client_Fired_Beams: {
                        on_receive_client_fired_beams(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Client_Fired_Torpedos: {
                        on_receive_client_fired_torpedos(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Client_Changed_Target: {
                        on_receive_target_changed(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial: {
                        on_receive_create_deep_space_anchor_initial(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Anchor_Creation: {
                        on_receive_create_deep_space_anchor(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Ship_Health_Update: {
                        on_receive_health_update(basePacket, map);
                        break;
                    }case PacketType::Server_To_Client_Ship_Cloak_Update: {
                        on_receive_cloak_update(basePacket, map);
                        break;
                    }case PacketType::Server_Shutdown: {
                        on_receive_server_shutdown(basePacket, menu, map);
                        break;
                    }default: {
                        break;
                    }
                }
            }
            switch (basePacket.PacketType) {
                case PacketType::Server_To_Client_Request_GameplayMode: {
                    on_receive_server_game_mode(basePacket);
                    break;
                }case PacketType::Server_To_Client_New_Client_Entered_Map: {
                    on_receive_new_client_entered_map(basePacket);
                    break;
                }case PacketType::Server_To_Client_Approve_Map_Entry: {
                    on_receive_server_approve_map_entry(basePacket, menu);
                    break;
                }case PacketType::Server_To_Client_Reject_Map_Entry: {
                    break;
                }case PacketType::Server_To_Client_Map_Data: {
                    on_receive_map_data(basePacket, menu);
                    break;
                }case PacketType::Server_To_Client_Chat_Message: {
                    on_receive_chat_message(basePacket, menu);
                    break;
                }case PacketType::Server_To_Client_Client_Joined_Server: {
                    on_receive_client_just_joined_server_lobby(basePacket, menu);
                    break;
                }case PacketType::Server_To_Client_Client_Left_Server:{
                    on_receive_client_just_left_server(basePacket, menu);
                    break;
                }case PacketType::Server_To_Client_Accept_Connection: {
                    on_receive_connection_accepted_by_server(basePacket, menu);
                    break;
                }case PacketType::Server_To_Client_Reject_Connection: {
                    on_receive_connection_rejected_by_server(basePacket, menu);
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(basePacket_Ptr);
    }
}
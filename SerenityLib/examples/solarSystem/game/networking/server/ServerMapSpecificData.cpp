#include "ServerMapSpecificData.h"

#include "../../ships/Ships.h"
#include "../../networking/packets/Packets.h"
#include "../../networking/server/Server.h"
#include "../../Helper.h"
#include "../../map/Map.h"
#include "../../map/Anchor.h"

using namespace std;

const double TIMER_DEEP_SPACE_ANCHOR_SPAM = 1.0;

#pragma region RespawningShips
ShipRespawning::ShipRespawning(Server& server) :m_Server(server) {

}
ShipRespawning::~ShipRespawning() {
    cleanup();
}
void ShipRespawning::processShip(const string& shipMapKey, const string& shipClass, const string& closest_spawn_anchor) {
    if (!m_Ships.count(shipMapKey)) {
        m_Ships.emplace(shipMapKey, make_tuple(shipClass, closest_spawn_anchor, Ships::Database[shipClass].RespawnTime));
    }else{
        //this should really not happen if cleanup() is implemented, but just to be safe...
        m_Ships.erase(shipMapKey);
        m_Ships.emplace(shipMapKey, make_tuple(shipClass, closest_spawn_anchor, Ships::Database[shipClass].RespawnTime));
    }
    auto& client = *m_Server.getClientByMapKey(shipMapKey);

    PacketShipRespawnNotification pOut;
    pOut.PacketType     = PacketType::Server_To_Client_Notify_Ship_Of_Impending_Respawn;
    pOut.timer          = static_cast<float>(Ships::Database[shipClass].RespawnTime);
    m_Server.send_to_client(client, pOut);
}
void ShipRespawning::cleanup() {
    for (auto it = m_Ships.begin(); it != m_Ships.end();) {
        auto& tuple = it->second;
        if (std::get<2>(tuple) <= 0.0) {
            it = m_Ships.erase(it);
        }else{
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
                const auto x = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
                const auto y = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
                const auto z = Helper::GetRandomFloatFromTo(-400.0f, 400.0f);
                const auto respawnPosition = glm::vec3(x, y, z);

                PacketShipRespawned pOut;
                pOut.PacketType = PacketType::Server_To_Client_Notify_Ship_Of_Respawn;
                pOut.respawned_ship_map_key = it.first; //mapkey
                pOut.x = respawnPosition.x;
                pOut.y = respawnPosition.y;
                pOut.z = respawnPosition.z;
                pOut.nearest_spawn_anchor_name = std::get<1>(tuple); //closest_spawn_anchor
                m_Server.send_to_all(pOut);

                //std::cout << "Processing Ship: " << it.first << " (" << std::get<0>(tuple) << ") may now respawn\n";
            }
        }
    }
    cleanup();
}
#pragma endregion

#pragma region CollisionEntries
CollisionEntries::CollisionEntries(Server& server) :m_Server(server) {

}
CollisionEntries::~CollisionEntries() {
    cleanup();
}
void CollisionEntries::internal_send_packet(const PacketCollisionEvent& packet_in) {
    PacketCollisionEvent pOut(packet_in);
    pOut.PacketType = PacketType::Server_To_Client_Collision_Event;
    m_Server.send_to_all(pOut);
}
void CollisionEntries::processCollision(const PacketCollisionEvent& packet_in, Map& map) {
    const string  key1     = packet_in.owner_key + "|" + packet_in.other_key;
    const string  key2     = packet_in.other_key + "|" + packet_in.owner_key;
    const bool    is_key_1 = m_CollisionPairs.count(key1);
    const bool    is_key_2 = m_CollisionPairs.count(key2);

    if (!is_key_1 && !is_key_2) {
        m_CollisionPairs.emplace(key1, 2.0);
        internal_send_packet(packet_in);
    }else{
        if (is_key_1) {
            if (m_CollisionPairs[key1] <= 0.0) {
                m_CollisionPairs[key1] = 2.0;
                internal_send_packet(packet_in);
            }
        }else if (is_key_2) {
            if (m_CollisionPairs[key2] <= 0.0) {
                m_CollisionPairs[key2] = 2.0;
                internal_send_packet(packet_in);
            }
        }
    }
}
void CollisionEntries::cleanup() {
    for (auto it = m_CollisionPairs.begin(); it != m_CollisionPairs.end();) {
        if (it->second <= 0.0) {
            it = m_CollisionPairs.erase(it);
        }else{
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

ServerMapSpecificData::ServerMapSpecificData(Server& server) : m_Server(server), m_CollisionEntries(server), m_RespawningShips(server){
    cleanup();
}
ServerMapSpecificData::~ServerMapSpecificData() {
    cleanup();
}
void ServerMapSpecificData::cleanup() {
    m_CollisionEntries.cleanup();
    m_RespawningShips.cleanup();
    m_DeepspaceAnchorTimer = 0.0;
    m_PhysicsUpdateTimer = 0.0;
    m_Map = nullptr;
}
void ServerMapSpecificData::internal_process_deepspace_anchor(Packet* packet_in) {
    if (m_DeepspaceAnchorTimer > TIMER_DEEP_SPACE_ANCHOR_SPAM) {
        //just forward it
        PacketMessage& pI = *static_cast<PacketMessage*>(packet_in);
        PacketMessage pOut(pI);
        pOut.PacketType = PacketType::Server_To_Client_Anchor_Creation;
        m_Server.send_to_all(pOut);
        m_DeepspaceAnchorTimer = 0.0;
    }
}
void ServerMapSpecificData::update(const double& dt) {
    m_DeepspaceAnchorTimer += dt;
    m_PhysicsUpdateTimer += dt;
    if (m_PhysicsUpdateTimer > PHYSICS_PACKET_TIMER_LIMIT) {
        auto& map = *m_Map;
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
            m_Server.send_to_all_but_client_udp(*m_Server.m_OwnerClient, p);
        }
        m_PhysicsUpdateTimer = 0.0;
    }
    m_CollisionEntries.update(dt);
    m_RespawningShips.update(dt);
}
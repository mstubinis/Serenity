#include "ClientMapSpecificData.h"
#include "Client.h"
#include "../packets/Packets.h"
#include "../../map/Map.h"
#include "../../map/Anchor.h"
#include "../../Ship.h"
#include "../../Helper.h"

#include <glm/gtx/norm.hpp>
#include <core/engine/math/Engine_Math.h>

using namespace std;
using namespace Engine;

const double DISTANCE_CHECK_NEAREST_ANCHOR       = 1000000.0 * 1000000.0;
const double DISTANCE_CHECK_NEAREST_OTHER_PLAYER = 100000.0  * 100000.0;

ClientMapSpecificData::ClientMapSpecificData(Client& client) : m_Client(client){
    m_Team         = nullptr;
    m_Map          = nullptr;
    m_PingTimeHealthUpdate = 0.0;
    m_PingTimePhysicsUpdate = 0.0;
}
ClientMapSpecificData::~ClientMapSpecificData() {
    cleanup();
}
void ClientMapSpecificData::cleanup() {
}
void ClientMapSpecificData::update(const double& dt) {
    m_PingTimeHealthUpdate += dt;
    m_PingTimePhysicsUpdate += dt;
    if (m_PingTimeHealthUpdate > PACKET_HEALTH_UPDATE_FREQUENCY) {
        auto& map    = *m_Map;
        auto& player = *map.getPlayer();

        //health status
        PacketHealthUpdate pOut2(player);
        pOut2.PacketType       = PacketType::Client_To_Server_Ship_Health_Update;
        m_Client.send(pOut2);
        m_PingTimeHealthUpdate = 0.0;
    }
    if (m_PingTimePhysicsUpdate > PHYSICS_PACKET_TIMER_LIMIT) {
        auto& map = *m_Map;
        //keep pinging the server, sending your ship physics info
        auto& playerShip = *map.getPlayer();

        Anchor* finalAnchor = map.getRootAnchor();
        const auto& anchor_list = map.getClosestAnchor();
        for (auto& closest : anchor_list) {
            finalAnchor = finalAnchor->getChild(closest);
        }
        PacketPhysicsUpdate p(playerShip, map, finalAnchor, anchor_list, m_Client.m_Username);
        p.PacketType = PacketType::Client_To_Server_Ship_Physics_Update;
        m_Client.send_udp(p);

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
                        pOut.data = to_string(anchor_list.size());
                        for (auto& closest : anchor_list) {
                            pOut.data += "," + closest;
                        }
                        //we want to create an anchor at r,g,b (the midpoint between two nearby ships), we send the nearest valid anchor as a reference
                        m_Client.send(pOut);
                        break;
                    }
                }
            }
        }
        m_PingTimePhysicsUpdate = 0.0;
    }
}
Map& ClientMapSpecificData::getMap() {
    return *m_Map;
}
Team& ClientMapSpecificData::getTeam() {
    return *m_Team;
}
Client& ClientMapSpecificData::getClient() {
    return m_Client;
}
GameplayMode& ClientMapSpecificData::getGameplayMode() {
    return m_GameplayMode;
}
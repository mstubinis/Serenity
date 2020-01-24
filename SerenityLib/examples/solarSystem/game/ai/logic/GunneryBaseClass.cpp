#include "GunneryBaseClass.h"
#include "../AI.h"
#include "../ThreatTable.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../networking/packets/PacketMessage.h"
#include "../../ships/Ships.h"
#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"

#include <core/engine/math/Engine_Math.h>

#include <algorithm>

using namespace std;
using namespace Engine;

GunneryBaseClass::GunneryBaseClass(AIType::Type type, AI& ai, ThreatTable& threatTable, Ship& ship, Map& map, ShipSystemSensors& sensors, ShipSystemWeapons& weapons) : m_AIType(type), m_AI(ai), m_ThreatTable(threatTable), m_Ship(ship), m_Map(map), m_Sensors(sensors), m_Weapons(weapons){
    std::random_device rd;
    m_RandomDevice  = std::mt19937(rd());
    m_BeamTimer     = 0.0;
    m_CannonTimer   = 0.0;
    m_TorpedoTimer  = 0.0;
}
GunneryBaseClass::~GunneryBaseClass() {

}
void GunneryBaseClass::internal_execute_beams(const double& dt) {
    PacketMessage pOut;
    decimal dist;
    glm_vec3 hull_point_world_pos, hull_point_local_pos, pos = glm_vec3(0.0);
    glm_quat rot;
    string hostile_ship_MapKey;
    int acquired_index = -1;
    for (auto& weapons : m_Weapons.getBeams()) {
        auto& weapon_ = *weapons.beam;
        for (auto& hostile_ship_ptr : m_Sensors.getEnemyShips()) {
            auto& hostile_ship  = *hostile_ship_ptr.ship;
            hostile_ship_MapKey = hostile_ship.getMapKey();
            pos = hostile_ship.getPosition();
            rot = hostile_ship.getRotation();
            dist       = weapon_.getDistanceSquared(pos);

            auto lamda_2 = [&](Ship& hostile, const glm_vec3& hostile_pos, const glm_quat& hostile_rotation, const decimal& hostile_distance) {
                if (weapon_.isInArc(hostile_pos, weapon_.arc)) {
                    auto& shipModelComponent = *hostile.getComponent<ComponentModel>();
                    if (hostile_distance < weapon_.rangeInKMSquared + static_cast<decimal>(shipModelComponent.radius()) * static_cast<decimal>(1.1)) {
                        auto hull_impact_points = Ships::Database[hostile.getClass()].HullImpactPoints;
                        decimal dist_to_impact_point;
                        std::shuffle(hull_impact_points.begin(), hull_impact_points.end(), m_RandomDevice);
                        for (size_t i = 0; i < hull_impact_points.size(); ++i) {
                            hull_point_local_pos = Math::rotate_vec3(hostile_rotation, hull_impact_points[i]);
                            hull_point_world_pos = hostile_pos + hull_point_local_pos;
                            dist_to_impact_point = weapon_.getDistanceSquared(hull_point_world_pos);
                            if (dist_to_impact_point < weapon_.rangeInKMSquared) {
                                if (weapon_.isInArc(hull_point_world_pos, weapon_.arc)) {
                                    acquired_index = weapon_.acquire_index();
                                    if (acquired_index >= 0) {
                                        if (pOut.data.empty()) {
                                            weapon_.setTarget(&hostile);
                                            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
                                            pOut.name = m_Ship.getName();
                                            pOut.r = static_cast<float>(hull_point_local_pos.r);
                                            pOut.g = static_cast<float>(hull_point_local_pos.g);
                                            pOut.b = static_cast<float>(hull_point_local_pos.b);
                                            pOut.data += to_string(weapon_.index) + "," + hostile.getName();
                                        }else{
                                            pOut.data += "," + to_string(weapon_.index) + "," + hostile.getName();
                                        }
                                        return true;
                                    }
                                    return false;
                                }
                            }
                        }
                    }
                }
                return false;
            };
            if (!hostile_ship.isDestroyed()) {
                if (m_ThreatTable.getShipsWithThreat().size() > 0) {
                    for (auto& ships_with_threat_pair : m_ThreatTable.getShipsWithThreat()) {
                        if (hostile_ship_MapKey == ships_with_threat_pair.first) {
                            const auto res1 = lamda_2(hostile_ship, pos, rot, dist);
                            if (res1)
                                break;
                        }
                    }
                }else{
                    const auto res1 = lamda_2(hostile_ship, pos, rot, dist);
                }
            }
        }
        if (!pOut.data.empty()) {
            m_Ship.m_Client.send(pOut);
        }
    }
}
void GunneryBaseClass::internal_execute_cannons(const double& dt) {
    decimal dist_to_i, dist_to_enemy;
    glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
    glm_quat enemy_ship_rot;
    string hostile_ship_MapKey;
    int acquired_index = -1;
    bool res1 = false;
    for (auto& weapons : m_Weapons.getCannons()) {
        auto& weapon_ = *weapons.cannon;
        PacketMessage pOut;
        for (auto& hostile_ship_ptr : m_Sensors.getEnemyShips()) {
            auto& hostile_ship = *hostile_ship_ptr.ship;
            hostile_ship_MapKey = hostile_ship.getMapKey();
            enemy_ship_pos = hostile_ship.getPosition();
            dist_to_enemy = weapon_.getDistanceSquared(enemy_ship_pos);
            enemy_ship_rot = hostile_ship.getRotation();
            auto lamda_2 = [&](Ship& hostile) {
                if (weapon_.isInArc(enemy_ship_pos, weapon_.arc)) {
                    if (dist_to_enemy < weapon_.rangeInKMSquared + static_cast<decimal>(hostile.getComponent<ComponentModel>()->radius()) * static_cast<decimal>(1.1)) {
                        auto pts = Ships::Database[hostile.getClass()].HullImpactPoints;
                        std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                        for (size_t i = 0; i < pts.size(); ++i) {
                            offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                            world_pos = enemy_ship_pos + offset;
                            dist_to_i = weapon_.getDistanceSquared(world_pos);
                            if (dist_to_i < weapon_.rangeInKMSquared) {
                                if (weapon_.isInArc(world_pos, weapon_.arc)) {
                                    acquired_index = weapon_.acquire_index();
                                    if (acquired_index >= 0) {
                                        if (pOut.data.empty()) {
                                            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Cannons;
                                            pOut.name = m_Ship.getName();
                                            pOut.r = static_cast<float>(offset.x);
                                            pOut.g = static_cast<float>(offset.y);
                                            pOut.b = static_cast<float>(offset.z);
                                            pOut.data += to_string(weapon_.index) + "," + to_string(acquired_index) + "," + hostile.getName();
                                        }else{
                                            pOut.data += "," + to_string(weapon_.index) + "," + to_string(acquired_index) + "," + hostile.getName();
                                        }
                                        return true;
                                    }
                                    return false;
                                }
                            }
                        }
                    }
                }
                return false;
            };
            if (!hostile_ship.isDestroyed()) {
                if (m_ThreatTable.getShipsWithThreat().size() > 0) {
                    for (auto& ships_with_threat : m_ThreatTable.getShipsWithThreat()) {
                        if (hostile_ship_MapKey == ships_with_threat.first) {
                            res1 = lamda_2(hostile_ship);
                            if (res1)
                                break;
                        }
                    }
                }else{
                    res1 = lamda_2(hostile_ship);
                }
            }
        }
        if (!pOut.data.empty()) {
            m_Ship.m_Client.send(pOut);
        }
    }
}
void GunneryBaseClass::internal_execute_torpedos(const double& dt) {
    decimal dist_to_i, dist_to_enemy;
    glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
    glm_quat enemy_ship_rot;
    string hostile_ship_MapKey;
    bool res1 = false;
    int acquired_index = -1;
    for (auto& weapons : m_Weapons.getTorpedos()) {
        auto& weapon_ = *weapons.torpedo;
        for (auto& hostile_ship_ptr : m_Sensors.getEnemyShips()) {
            auto& hostile_ship = *hostile_ship_ptr.ship;
            hostile_ship_MapKey = hostile_ship.getMapKey();
            enemy_ship_pos = hostile_ship.getPosition();
            dist_to_enemy = weapon_.getDistanceSquared(enemy_ship_pos);
            enemy_ship_rot = hostile_ship.getRotation();

            auto lamda_2 = [&](Ship& hostile) {
                if (weapon_.isInArc(enemy_ship_pos, weapon_.arc)) {
                    if (dist_to_enemy < weapon_.rangeInKMSquared + static_cast<decimal>(hostile.getComponent<ComponentModel>()->radius()) * static_cast<decimal>(1.1)) {
                        auto pts = Ships::Database[hostile.getClass()].HullImpactPoints;
                        std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                        for (size_t i = 0; i < pts.size(); ++i) {
                            offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                            world_pos = enemy_ship_pos + offset;
                            dist_to_i = weapon_.getDistanceSquared(world_pos);
                            if (dist_to_i < weapon_.rangeInKMSquared) {
                                if (weapon_.isInArc(world_pos, weapon_.arc)) {
                                    acquired_index = weapon_.acquire_index();
                                    if (acquired_index >= 0) {
                                        PacketMessage pOut;
                                        if (pOut.data.empty()) {
                                            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Torpedos;
                                            pOut.name = m_Ship.getName();
                                            pOut.r = static_cast<float>(offset.x);
                                            pOut.g = static_cast<float>(offset.y);
                                            pOut.b = static_cast<float>(offset.z);
                                            pOut.data += to_string(weapon_.index) + "," + to_string(acquired_index) + "," + hostile.getName();
                                        }else{
                                            pOut.data += "," + to_string(weapon_.index) + "," + to_string(acquired_index) + "," + hostile.getName();
                                        }
                                        if (!pOut.data.empty()) {
                                            m_Ship.m_Client.send(pOut);
                                        }
                                        return true;
                                    }
                                    return false;
                                }
                            }
                        }
                    }
                }
                return false;
            };
            if (!hostile_ship.isDestroyed()) {
                if (m_ThreatTable.getShipsWithThreat().size() > 0) {
                    for (auto& ships_with_threat : m_ThreatTable.getShipsWithThreat()) {
                        if (hostile_ship_MapKey == ships_with_threat.first) {
                            res1 = lamda_2(hostile_ship);
                            if (res1) {
                                m_TorpedoTimer = 0.0;
                                return;
                            }
                        }
                    }
                }else{
                    res1 = lamda_2(hostile_ship);
                    if (res1) {
                        m_TorpedoTimer = 0.0;
                        return;
                    }
                }
            }
        }
    }
}
void GunneryBaseClass::update(const double& dt) {
    /*
    TODO: consider optimizations here
    */
    m_BeamTimer += dt;
    m_CannonTimer += dt;
    m_TorpedoTimer += dt;

    if (m_BeamTimer > 0.4) {
        internal_execute_beams(dt);
        m_BeamTimer = 0.0;
    }
    if (m_CannonTimer > 0.5) {
        internal_execute_cannons(dt);
        m_CannonTimer = 0.0;
    }
    if (m_TorpedoTimer > 0.5) {
        internal_execute_torpedos(dt);
        m_TorpedoTimer = 0.0;
    }
}
#include "GunneryBaseClass.h"
#include "../AI.h"
#include "../ThreatTable.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"
#include <core/engine/math/Engine_Math.h>
#include "../../networking/Packet.h"

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

void GunneryBaseClass::update(const double& dt) {
    /*
    TODO: consider optimizations here

    for each weapon that is available to fire:
         - get a list of hostile ships in range
             - find the greatest threat ship within this hostile list of ships and fire at it
                 - if no greatest threat ship, fire at a random hostile ship (change to nearest hostile)


    */
    m_BeamTimer += dt;
    m_CannonTimer += dt;
    m_TorpedoTimer += dt;

    auto lamda_beams = [&](PrimaryWeaponBeam& weapon_, ShipSystemSensors& sensors_, PacketMessage& pOut_) {
        decimal dist_to_i, dist_to_enemy;
        glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
        glm_quat enemy_ship_rot;

        for (auto& hostile_ship : sensors_.getEnemyShips()) {
            auto& ship                = *hostile_ship.ship;
            const auto& shipMapKey    = ship.getMapKey();
            enemy_ship_pos            = ship.getPosition();
            dist_to_enemy             = weapon_.getDistanceSquared(enemy_ship_pos);
            enemy_ship_rot            = ship.getRotation();

            auto lamda_2 = [&]() {
                if (!ship.isDestroyed()) {
                    if (weapon_.isInArc(enemy_ship_pos, weapon_.arc)) {
                        if (dist_to_enemy < weapon_.rangeInKMSquared + static_cast<decimal>(ship.getComponent<ComponentModel>()->radius()) * static_cast<decimal>(1.1)) {
                            auto pts = ship.m_AimPositionDefaults;
                            std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                            for (unsigned int i = 0; i < pts.size(); ++i) {
                                offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                                world_pos = enemy_ship_pos + offset;
                                dist_to_i = weapon_.getDistanceSquared(world_pos);
                                if (dist_to_i < weapon_.rangeInKMSquared) {
                                    if (weapon_.isInArc(world_pos, weapon_.arc)) {
                                        int res = weapon_.acquire_index();
                                        if (res >= 0) {
                                            weapon_.setTarget(&ship);
                                            pOut_.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
                                            pOut_.name = m_Ship.getName();
                                            pOut_.r = static_cast<float>(offset.r);
                                            pOut_.g = static_cast<float>(offset.g);
                                            pOut_.b = static_cast<float>(offset.b);
                                            if (pOut_.data.empty()) {
                                                pOut_.data += to_string(weapon_.index) + "," + ship.getName();
                                            }else{
                                                pOut_.data += "," + to_string(weapon_.index) + "," + ship.getName();
                                            }
                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                return false;
            };

            if(m_ThreatTable.getShipsWithThreat().size() > 0){
                for (auto& ships_with_threat : m_ThreatTable.getShipsWithThreat()) {
                    if (shipMapKey == ships_with_threat.first) {
                        const auto res1 = lamda_2();
                        if (res1) {
                            return true;
                        }
                    }
                }
            }else{
                const auto res1 = lamda_2();
                if (res1) {
                    return true;
                }
            }
        }
        return false;
    }; 
    auto lamda_cannons = [&](PrimaryWeaponCannon& weapon_, ShipSystemSensors& sensors_, PacketMessage& pOut_) {
        decimal dist_to_i, dist_to_enemy;
        glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
        glm_quat enemy_ship_rot;

        for (auto& hostile_ship : sensors_.getEnemyShips()) {
            auto& ship             = *hostile_ship.ship;
            const auto& shipMapKey = ship.getMapKey();
            enemy_ship_pos         = ship.getPosition();
            dist_to_enemy          = weapon_.getDistanceSquared(enemy_ship_pos);
            enemy_ship_rot         = ship.getRotation();

            auto lamda_2 = [&]() {
                if (!ship.isDestroyed()) {
                    if (weapon_.isInArc(enemy_ship_pos, weapon_.arc)) {
                        if (dist_to_enemy < weapon_.rangeInKMSquared + static_cast<decimal>(ship.getComponent<ComponentModel>()->radius()) * static_cast<decimal>(1.1)) {
                            auto pts = ship.m_AimPositionDefaults;
                            std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                            for (unsigned int i = 0; i < pts.size(); ++i) {
                                offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                                world_pos = enemy_ship_pos + offset;
                                dist_to_i = weapon_.getDistanceSquared(world_pos);
                                if (dist_to_i < weapon_.rangeInKMSquared) {
                                    if (weapon_.isInArc(world_pos, weapon_.arc)) {
                                        int res = weapon_.acquire_index();
                                        if (res >= 0) {
                                            pOut_.PacketType = PacketType::Client_To_Server_Client_Fired_Cannons;
                                            pOut_.name = m_Ship.getName();
                                            pOut_.r = static_cast<float>(offset.x);
                                            pOut_.g = static_cast<float>(offset.y);
                                            pOut_.b = static_cast<float>(offset.z);
                                            if (pOut_.data.empty()) {
                                                pOut_.data += to_string(weapon_.index) + "," + to_string(res) + "," + ship.getName();
                                            }else{
                                                pOut_.data += "," + to_string(weapon_.index) + "," + to_string(res) + "," + ship.getName();
                                            }
                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                return false;
            };   
            if (m_ThreatTable.getShipsWithThreat().size() > 0) {
                for (auto& ships_with_threat : m_ThreatTable.getShipsWithThreat()) {
                    if (shipMapKey == ships_with_threat.first) {
                        const auto res1 = lamda_2();
                        if (res1) {
                            return true;
                        }
                    }
                }
            }else{
                const auto res1 = lamda_2();
                if (res1) {
                    return true;
                }
            }
        }
        return false;
    };
    
    auto lamda_torpedos = [&](SecondaryWeaponTorpedo& weapon_, ShipSystemSensors& sensors_, PacketMessage& pOut_) {
        decimal dist_to_i, dist_to_enemy;
        glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
        glm_quat enemy_ship_rot;

        for (auto& hostile_ship : sensors_.getEnemyShips()) {
            auto& ship             = *hostile_ship.ship;
            const auto& shipMapKey = ship.getMapKey();
            enemy_ship_pos         = ship.getPosition();
            dist_to_enemy          = weapon_.getDistanceSquared(enemy_ship_pos);
            enemy_ship_rot         = ship.getRotation();

            auto lamda_2 = [&]() {
                if (!ship.isDestroyed()) {
                    if (weapon_.isInArc(enemy_ship_pos, weapon_.arc)) {
                        if (dist_to_enemy < weapon_.rangeInKMSquared + static_cast<decimal>(ship.getComponent<ComponentModel>()->radius()) * static_cast<decimal>(1.1)) {
                            auto pts = ship.m_AimPositionDefaults;
                            std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                            for (unsigned int i = 0; i < pts.size(); ++i) {
                                offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                                world_pos = enemy_ship_pos + offset;
                                dist_to_i = weapon_.getDistanceSquared(world_pos);
                                if (dist_to_i < weapon_.rangeInKMSquared) {
                                    if (weapon_.isInArc(world_pos, weapon_.arc)) {
                                        int res = weapon_.acquire_index();
                                        if (res >= 0) {
                                            PacketMessage pOut;
                                            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Torpedos;
                                            pOut.name = m_Ship.getName();
                                            pOut.r = static_cast<float>(offset.x);
                                            pOut.g = static_cast<float>(offset.y);
                                            pOut.b = static_cast<float>(offset.z);
                                            if (pOut.data.empty()) {
                                                pOut.data += to_string(weapon_.index) + "," + to_string(res) + "," + ship.getName();
                                            }else{
                                                pOut.data += "," + to_string(weapon_.index) + "," + to_string(res) + "," + ship.getName();
                                            }
                                            if (!pOut.data.empty()) {
                                                m_Ship.m_Client.send(pOut);
                                            }
                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                return false;
            };
            if (m_ThreatTable.getShipsWithThreat().size() > 0) {
                for (auto& ships_with_threat : m_ThreatTable.getShipsWithThreat()) {
                    if (shipMapKey == ships_with_threat.first) {
                        const auto res1 = lamda_2();
                        if (res1) {
                            return true;
                        }
                    }
                }
            }else{
                const auto res1 = lamda_2();
                if (res1) {
                    return true;
                }
            }
        }
        return false;
    };

    
    if (m_BeamTimer > 0.4) {
        PacketMessage pOut;
        for (auto& weapons : m_Weapons.getBeams()) {
            auto& weapon = *weapons.beam;
            const auto res = lamda_beams(weapon, m_Sensors, pOut);
            if (!pOut.data.empty()) {
                m_Ship.m_Client.send(pOut);
            }
        }
        m_BeamTimer = 0.0;
    }
    if (m_CannonTimer > 0.5) {
        PacketMessage pOut;
        for (auto& weapons : m_Weapons.getCannons()) {
            auto& weapon = *weapons.cannon;
            const auto res = lamda_cannons(weapon, m_Sensors, pOut);
            if (!pOut.data.empty()) {
                m_Ship.m_Client.send(pOut);
            }
        }
        m_CannonTimer = 0.0;
    }
    if (m_TorpedoTimer > 0.5) {
        PacketMessage pOut;
        for (auto& weapons : m_Weapons.getTorpedos()) {
            auto& weapon = *weapons.torpedo;
            const auto res = lamda_torpedos(weapon, m_Sensors, pOut);
            if (res) {
                m_TorpedoTimer = 0.0;
                return;
            }
        }
        m_TorpedoTimer = 0.0;
    }
}
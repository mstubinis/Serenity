#include "FireAtWill.h"
#include <core/engine/math/Engine_Math.h>

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "../ships/shipSystems/ShipSystemSensors.h"
#include "../map/Map.h"
#include "../Ship.h"
#include "../networking/Packet.h"
#include "../networking/Client.h"

#include <algorithm>

using namespace std;
using namespace Engine;

FireAtWill::FireAtWill(const AIType::Type& type, Ship& ship, Map& map, ShipSystemSensors& sensors, ShipSystemWeapons& weapons) : m_Ship(ship), m_Map(map), m_Sensors(sensors), m_Weapons(weapons){
    internal_reset_timers();

    std::random_device rd;
    m_RandomDevice = std::mt19937(rd());

    if (type == AIType::Player_You || type == AIType::Player_Other) {
        m_IsUsingForwardWeapons = false;
        m_Activated = false;
    }else{
        m_IsUsingForwardWeapons = true;
        m_Activated = true;
    }
}
FireAtWill::~FireAtWill() {

}
void FireAtWill::internal_execute_beams() {
    decimal dist_to_i, dist_to_enemy;
    glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
    glm_quat enemy_ship_rot;
    int res;

    auto lamda = [&](Ship& enemyShip, PrimaryWeaponBeam& beam, PacketMessage& pOut) {
        if (enemyShip.isDestroyed())
            return false;
        enemy_ship_pos = enemyShip.getPosition();
        if (beam.isInArc(enemy_ship_pos, beam.arc)) {
            res = beam.canFire();
            if (res >= 0) {
                enemy_ship_rot = enemyShip.getRotation();
                dist_to_enemy = beam.getDistanceSquared(enemy_ship_pos);
                if (dist_to_enemy < beam.rangeInKMSquared + static_cast<decimal>(enemyShip.getComponent<ComponentModel>()->radius()) * static_cast<decimal>(1.1)) {
                    auto pts = enemyShip.m_AimPositionDefaults;
                    std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                    for (unsigned int i = 0; i < pts.size(); ++i) {
                        offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                        world_pos = enemy_ship_pos + offset;
                        dist_to_i = beam.getDistanceSquared(world_pos);
                        if (dist_to_i < beam.rangeInKMSquared) {
                            if (beam.isInArc(world_pos, beam.arc)) {
                                beam.setTarget(&enemyShip);
                                pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
                                pOut.name = m_Ship.getName();
                                pOut.r = static_cast<float>(offset.r);
                                pOut.g = static_cast<float>(offset.g);
                                pOut.b = static_cast<float>(offset.b);

                                if (pOut.data.empty()) {
                                    pOut.data += to_string(beam.index) + "," + enemyShip.getName();
                                }else{
                                    pOut.data += "," + to_string(beam.index) + "," + enemyShip.getName();
                                }
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    };
    auto lamda_execute = [&](std::vector<ShipSystemWeapons::WeaponBeam>& beams) {
        PacketMessage pOut;
        for (auto& beam_ptr : beams) {
            auto& beam = *beam_ptr.beam;
            for (auto& enemy : m_Sensors.getEnemyShips()) {
                auto& enemyShip = *enemy.ship;
                const auto res = lamda(enemyShip, beam, pOut);
            }
            for (auto& enemy : m_Sensors.getAntiCloakDetectedShips()) {
                auto& enemyShip = *enemy.ship;
                const auto res = lamda(enemyShip, beam, pOut);
            }
        }
        if (!pOut.data.empty())
            m_Ship.m_Client.send(pOut);
    };
    if (!m_IsUsingForwardWeapons) {
        lamda_execute(m_Weapons.getNonForwardBeams());
    }else{
        lamda_execute(m_Weapons.getBeams());
    }
}
void FireAtWill::internal_execute_cannons() {
    decimal dist_to_i, dist_to_enemy;
    glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
    glm_quat enemy_ship_rot;
    int res;

    auto lamda = [&](Ship& enemyShip, PrimaryWeaponCannon& cannon, PacketMessage& pOut) {
        if (enemyShip.isDestroyed())
            return false;
        enemy_ship_pos = enemyShip.getPosition();
        if (cannon.isInArc(enemy_ship_pos, cannon.arc)) {
            res = cannon.canFire();
            if (res >= 0) {
                enemy_ship_rot = enemyShip.getRotation();
                dist_to_enemy = cannon.getDistanceSquared(enemy_ship_pos);
                if (dist_to_enemy < 100.0 * 100.0) { //TODO: add range later?
                    auto pts = enemyShip.m_AimPositionDefaults;
                    std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                    for (unsigned int i = 0; i < pts.size(); ++i) {
                        offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                        world_pos = enemy_ship_pos + offset;
                        dist_to_i = cannon.getDistanceSquared(world_pos);
                        if (dist_to_i < 100.0 * 100.0) { //TODO: add range later?
                            if (cannon.isInArc(world_pos, cannon.arc)) {
                                pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Cannons;
                                pOut.name = m_Ship.getName();
                                pOut.r = static_cast<float>(offset.x);
                                pOut.g = static_cast<float>(offset.y);
                                pOut.b = static_cast<float>(offset.z);
                                if (pOut.data.empty()) {
                                    pOut.data += to_string(cannon.index) + "," + to_string(res) + "," + enemyShip.getName();
                                }else{
                                    pOut.data += "," + to_string(cannon.index) + "," + to_string(res) + "," + enemyShip.getName();
                                }
                                return true;
                            }
                        }

                    }
                }
            }
        }
        return false;
    };
    auto lamda_execute = [&](std::vector<ShipSystemWeapons::WeaponCannon>& cannons) {
        PacketMessage pOut;
        for (auto& ptr : cannons) {
            auto& weapon = *ptr.cannon;
            for (auto& enemy : m_Sensors.getEnemyShips()) {
                auto& enemyShip = *enemy.ship;
                const auto res = lamda(enemyShip, weapon, pOut);
            }
            for (auto& enemy : m_Sensors.getAntiCloakDetectedShips()) {
                auto& enemyShip = *enemy.ship;
                const auto res = lamda(enemyShip, weapon, pOut);
            }
        }
        if(!pOut.data.empty())
            m_Ship.m_Client.send(pOut);
    };
    if (!m_IsUsingForwardWeapons) {
        lamda_execute(m_Weapons.getNonForwardCannons());
    }else{
        lamda_execute(m_Weapons.getCannons());
    }
}
void FireAtWill::internal_execute_torpedos() {
    decimal dist_to_i, dist_to_enemy;
    glm_vec3 world_pos, offset, enemy_ship_pos = glm_vec3(0.0);
    glm_quat enemy_ship_rot;
    int res;

    auto lamda = [&](Ship& enemyShip, SecondaryWeaponTorpedo& torpedo) {
        if (enemyShip.isDestroyed())
            return false;
        enemy_ship_pos = enemyShip.getPosition();
        if (torpedo.isInArc(enemy_ship_pos, torpedo.arc)) {
            res = torpedo.canFire();
            if (res >= 0) {
                enemy_ship_rot = enemyShip.getRotation();
                dist_to_enemy = torpedo.getDistanceSquared(enemy_ship_pos);
                if (dist_to_enemy < 100.0 * 100.0) { //TODO: add range later?
                    auto pts = enemyShip.m_AimPositionDefaults;
                    std::shuffle(pts.begin(), pts.end(), m_RandomDevice);
                    for (unsigned int i = 0; i < pts.size(); ++i) {
                        offset = Math::rotate_vec3(enemy_ship_rot, pts[i]);
                        world_pos = enemy_ship_pos + offset;
                        dist_to_i = torpedo.getDistanceSquared(world_pos);
                        if (dist_to_i < 100.0 * 100.0) { //TODO: add range later?
                            if (torpedo.isInArc(world_pos, torpedo.arc)) {
                                PacketMessage pOut;
                                pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Torpedos;
                                pOut.name = m_Ship.getName();
                                pOut.r = static_cast<float>(offset.x);
                                pOut.g = static_cast<float>(offset.y);
                                pOut.b = static_cast<float>(offset.z);
                                if (pOut.data.empty()) {
                                    pOut.data += to_string(torpedo.index) + "," + to_string(res) + "," + enemyShip.getName();
                                }else{
                                    pOut.data += "," + to_string(torpedo.index) + "," + to_string(res) + "," + enemyShip.getName();
                                }
                                if (!pOut.data.empty())
                                    m_Ship.m_Client.send(pOut);
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    };
    auto lamda_execute = [&](std::vector<ShipSystemWeapons::WeaponTorpedo>& torpedos) {
        for (auto& ptr : torpedos) {
            auto& weapon = *ptr.torpedo;
            for (auto& enemy : m_Sensors.getEnemyShips()) {
                auto& enemyShip = *enemy.ship;
                const auto res = lamda(enemyShip, weapon);
                if (res)
                    return;
            }
            for (auto& enemy : m_Sensors.getAntiCloakDetectedShips()) {
                auto& enemyShip = *enemy.ship;
                const auto res = lamda(enemyShip, weapon);
                if (res)
                    return;
            }
        }
    };
    if (!m_IsUsingForwardWeapons) {
        lamda_execute(m_Weapons.getNonForwardTorpedos());
    }else{
        lamda_execute(m_Weapons.getTorpedos());
    }
}
void FireAtWill::internal_reset_timers() {
    m_TimerTorpedo = 0.0;
    m_TimerCannon  = 0.0;
    m_TimerBeam    = 0.0;
}
void FireAtWill::update(const double& dt) {
    if (!m_Activated || m_Ship.getAIType() == AIType::Player_Other)
        return;
    if (m_Ship.isCloaked() || m_Ship.IsWarping())
        return;
    m_TimerBeam += dt;
    m_TimerCannon += dt;
    m_TimerTorpedo += dt;
    if (m_Sensors.getEnemyShips().size() == 0 && m_Sensors.getAntiCloakDetectedShips().size() == 0)
        return;

    if (m_TimerBeam > 0.4) {
        internal_execute_beams();
        m_TimerBeam = 0.0;
    }
    if (m_TimerCannon > 0.5) {
        internal_execute_cannons();
        m_TimerCannon = 0.0;
    }
    if (m_TimerTorpedo > 0.5) {
        internal_execute_torpedos();
        m_TimerTorpedo = 0.0;
    }
}
void FireAtWill::activate() {
    m_Activated    = true;
    internal_reset_timers();
}
void FireAtWill::deactivate() {
    m_Activated    = false;
    internal_reset_timers();
}
void FireAtWill::toggle() {
    m_Activated    = !m_Activated;
    internal_reset_timers();
}
#include "Constitution.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/torpedos/PhotonTorpedoOld.h"
#include "../../weapons/beams/PhaserBeam.h"

#include "../../ships/shipSystems/ShipSystemCloakingDevice.h"
#include "../../ships/shipSystems/ShipSystemMainThrusters.h"
#include "../../ships/shipSystems/ShipSystemPitchThrusters.h"
#include "../../ships/shipSystems/ShipSystemReactor.h"
#include "../../ships/shipSystems/ShipSystemRollThrusters.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"
#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemWarpDrive.h"
#include "../../ships/shipSystems/ShipSystemYawThrusters.h"
#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include "../../ships/shipSystems/ShipSystemHull.h"
#include "../../ai/AI.h"
#include "../Ships.h"

constexpr auto CLASS = "Constitution";

using namespace std;

Constitution::Constitution(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS, map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this);
        else if (i == 4)  system = nullptr; //no cloaking device
        else if (i == 5)  system = new ShipSystemShields(_this, map, 11000.0f, 11000.0f, 11000.0f, 11000.0f, 14000.0f, 14000.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 20000.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTorp_1 = new PhotonTorpedoOld(_this, map, glm::vec3(-0.033864f, -0.129594f, -0.664163f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* rightTorp_1 = new PhotonTorpedoOld(_this, map, glm::vec3(0.033864f, -0.129594f, -0.664163f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* leftTorp_2 = new PhotonTorpedoOld(_this, map, glm::vec3(-0.033864f, -0.129594f, -0.664163f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* rightTorp_2 = new PhotonTorpedoOld(_this, map, glm::vec3(0.033864f, -0.129594f, -0.664163f), glm::vec3(0, 0, -1), 15.0f, 1);

    weapons.addSecondaryWeaponTorpedo(*leftTorp_1, true);
    weapons.addSecondaryWeaponTorpedo(*rightTorp_1, true);
    weapons.addSecondaryWeaponTorpedo(*leftTorp_2, true);
    weapons.addSecondaryWeaponTorpedo(*rightTorp_2, true);

    vector<glm::vec3> top_center_left_beam{ glm::vec3(-0.014609f, 0.303966f, -1.82334f) };
    vector<glm::vec3> top_center_right_beam{ glm::vec3(0.014609f, 0.303966f, -1.82334f) };

    vector<glm::vec3> top_left_left_beam{ glm::vec3(-0.450489f, 0.303966f, -1.3616f) };
    vector<glm::vec3> top_left_right_beam{ glm::vec3(-0.450489f, 0.303966f, -1.39465f) };

    vector<glm::vec3> top_right_left_beam{ glm::vec3(0.450489f, 0.303966f, -1.3616f) };
    vector<glm::vec3> top_right_right_beam{ glm::vec3(0.450489f, 0.303966f, -1.39465f) };

    vector<glm::vec3> aft_left_beam{ glm::vec3(-0.086977f, -0.395132f, 0.963498f) };
    vector<glm::vec3> aft_right_beam{ glm::vec3(0.086977f, -0.395132f, 0.963498f) };

    vector<glm::vec3> btm_center_left_beam{ glm::vec3(0.019838f, 0.104979f, -1.80528f) };
    vector<glm::vec3> btm_center_right_beam{ glm::vec3(-0.019838f, 0.104979f, -1.80528f) };

    vector<glm::vec3> btm_left_left_beam{ glm::vec3(0.423303f, 0.104979f, -1.36245f) };
    vector<glm::vec3> btm_left_right_beam{ glm::vec3(0.423303f, 0.104979f, -1.39817f) };

    vector<glm::vec3> btm_right_left_beam{ glm::vec3(-0.423303f, 0.104979f, -1.36245f) };
    vector<glm::vec3> btm_right_right_beam{ glm::vec3(-0.423303f, 0.104979f, -1.39817f) };

    auto* top_center_left_phaser = new PhaserBeam(_this, map, top_center_left_beam[0], glm::vec3(0.0f, 0.185146f, -0.286489f), 35.0f, top_center_left_beam);
    auto* top_center_right_phaser = new PhaserBeam(_this, map, top_center_right_beam[0], glm::vec3(0.0f, 0.185146f, -0.286489f), 35.0f, top_center_right_beam);
    auto* top_left_left_phaser = new PhaserBeam(_this, map, top_left_left_beam[0], glm::vec3(-0.286489, 0.185146f, 0.0f), 35.0f, top_left_left_beam);
    auto* top_left_right_phaser = new PhaserBeam(_this, map, top_left_right_beam[0], glm::vec3(-0.286489, 0.185146f, 0.0f), 35.0f, top_left_right_beam);
    auto* top_right_left_phaser = new PhaserBeam(_this, map, top_right_left_beam[0], glm::vec3(0.286489, 0.185146f, 0.0f), 35.0f, top_right_left_beam);
    auto* top_right_right_phaser = new PhaserBeam(_this, map, top_right_right_beam[0], glm::vec3(0.286489, 0.185146f, 0.0f), 35.0f, top_right_right_beam);
    auto* aft_left_phaser = new PhaserBeam(_this, map, aft_left_beam[0], glm::vec3(-0.019173f, 0.0, 0.122934f), 30.0f, aft_left_beam);
    auto* aft_right_phaser = new PhaserBeam(_this, map, aft_right_beam[0], glm::vec3(0.019173f, 0.0, 0.122934f), 30.0f, aft_right_beam);
    auto* btm_center_left_phaser = new PhaserBeam(_this, map, btm_center_left_beam[0], glm::vec3(0.0f, -0.219888f, -0.286489f), 35.0f, btm_center_left_beam);
    auto* btm_center_right_phaser = new PhaserBeam(_this, map, btm_center_right_beam[0], glm::vec3(0.0f, -0.219888f, -0.286489f), 35.0f, btm_center_right_beam);
    auto* btm_left_left_phaser = new PhaserBeam(_this, map, btm_left_left_beam[0], glm::vec3(0.286489f, -0.219888f, 0.0f), 35.0f, btm_left_left_beam);
    auto* btm_left_right_phaser = new PhaserBeam(_this, map, btm_left_right_beam[0], glm::vec3(0.286489f, -0.219888f, 0.0f), 35.0f, btm_left_right_beam);
    auto* btm_right_left_phaser = new PhaserBeam(_this, map, btm_right_left_beam[0], glm::vec3(-0.286489f, -0.219888f, 0.0f), 35.0f, btm_right_left_beam);
    auto* btm_right_right_phaser = new PhaserBeam(_this, map, btm_right_right_beam[0], glm::vec3(-0.286489f, -0.219888f, 0.0f), 35.0f, btm_right_right_beam);

    weapons.addPrimaryWeaponBeam(*top_center_left_phaser, true);
    weapons.addPrimaryWeaponBeam(*top_center_right_phaser, true);
    weapons.addPrimaryWeaponBeam(*top_left_left_phaser);
    weapons.addPrimaryWeaponBeam(*top_left_right_phaser);
    weapons.addPrimaryWeaponBeam(*top_right_left_phaser);
    weapons.addPrimaryWeaponBeam(*top_right_right_phaser);
    weapons.addPrimaryWeaponBeam(*aft_left_phaser);
    weapons.addPrimaryWeaponBeam(*aft_right_phaser);
    weapons.addPrimaryWeaponBeam(*btm_center_left_phaser, true);
    weapons.addPrimaryWeaponBeam(*btm_center_right_phaser, true);
    weapons.addPrimaryWeaponBeam(*btm_left_left_phaser);
    weapons.addPrimaryWeaponBeam(*btm_left_right_phaser);
    weapons.addPrimaryWeaponBeam(*btm_right_left_phaser);
    weapons.addPrimaryWeaponBeam(*btm_right_right_phaser);

    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 0.004208f, -0.383244f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Constitution::~Constitution() {

}
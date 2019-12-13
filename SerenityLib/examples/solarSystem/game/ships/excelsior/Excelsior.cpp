#include "Excelsior.h"
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

constexpr auto CLASS = "Excelsior";

using namespace std;

Excelsior::Excelsior(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS, map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = NEW ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = NEW ShipSystemYawThrusters(_this);
        else if (i == 3)  system = NEW ShipSystemRollThrusters(_this);
        else if (i == 4)  system = nullptr; //no cloaking device
        else if (i == 5)  system = NEW ShipSystemShields(_this, map, 15000.0f, 15000.0f, 15000.0f, 15000.0f, 19000.0f, 19000.0f);
        else if (i == 6)  system = NEW ShipSystemMainThrusters(_this);
        else if (i == 7)  system = NEW ShipSystemWarpDrive(_this);
        else if (i == 8)  system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)  system = NEW ShipSystemWeapons(_this);
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 26000.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTorp_1 = NEW PhotonTorpedoOld(_this, map, glm::vec3(-0.252308f, -0.294315f, -1.15895f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* rightTorp_1 = NEW PhotonTorpedoOld(_this, map, glm::vec3(0.252308f, -0.294315f, -1.15895f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* leftTorp_2 = NEW PhotonTorpedoOld(_this, map, glm::vec3(-0.252308f, -0.294315f, -1.15895f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* rightTorp_2 = NEW PhotonTorpedoOld(_this, map, glm::vec3(0.252308f, -0.294315f, -1.15895f), glm::vec3(0, 0, -1), 15.0f, 1);

    auto* leftTorpAft = NEW PhotonTorpedoOld(_this, map, glm::vec3(-0.151062f, -0.276966f, 2.24787f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* rightTorpAft = NEW PhotonTorpedoOld(_this, map, glm::vec3(0.151062f, -0.276966f, 2.24787f), glm::vec3(0, 0, 1), 15.0f, 1);

    weapons.addSecondaryWeaponTorpedo(*leftTorp_1, true);
    weapons.addSecondaryWeaponTorpedo(*rightTorp_1, true);
    weapons.addSecondaryWeaponTorpedo(*leftTorp_2, true);
    weapons.addSecondaryWeaponTorpedo(*rightTorp_2, true);
    weapons.addSecondaryWeaponTorpedo(*leftTorpAft);
    weapons.addSecondaryWeaponTorpedo(*rightTorpAft);

    //top saucer phasers
    //top central phaser
    vector<glm::vec3> top_left_central{ glm::vec3(-0.019191f, 0.297236f, -2.45391f) };
    vector<glm::vec3> top_right_central{ glm::vec3(0.019191f, 0.297236f, -2.45391f) };

    //top left phaser
    vector<glm::vec3> top_left_left{ glm::vec3(-0.409018f, 0.297236f, -2.27202f) };
    vector<glm::vec3> top_left_right{ glm::vec3(-0.384615f, 0.297236f, -2.3009f) };

    //top left-left phaser
    vector<glm::vec3> top_left_left_left{ glm::vec3(-0.525955f, 0.297236f, -1.84258f) };
    vector<glm::vec3> top_left_left_right{ glm::vec3(-0.531661f, 0.297236f, -1.88045f) };

    //top right phaser
    vector<glm::vec3> top_right_left{ glm::vec3(0.409018f, 0.297236f, -2.27202f) };
    vector<glm::vec3> top_right_right{ glm::vec3(0.384615f, 0.297236f, -2.3009f) };

    //top right-right phaser
    vector<glm::vec3> top_right_right_left{ glm::vec3(0.525955f, 0.297236f, -1.84258f) };
    vector<glm::vec3> top_right_right_right{ glm::vec3(0.531661f, 0.297236f, -1.88045f) };

    auto* topCentralPhaserLeft = NEW PhaserBeam(_this, map, top_left_central[0], glm::vec3(0.0f, 0.484739f, -0.25347f), 35.0f, top_left_central);
    auto* topCentralPhaserRight = NEW PhaserBeam(_this, map, top_right_central[0], glm::vec3(0.0f, 0.484739f, -0.25347f), 35.0f, top_right_central);

    auto* topLeftLeftPhaser = NEW PhaserBeam(_this, map, top_left_left[0], glm::vec3(-0.188111f, 0.484739f, -0.169887f), 35.0f, top_left_left);
    auto* topLeftRightPhaser = NEW PhaserBeam(_this, map, top_left_right[0], glm::vec3(-0.188111f, 0.484739f, -0.169887f), 35.0f, top_left_right);

    auto* topLeftLeftLeftPhaser = NEW PhaserBeam(_this, map, top_left_left_left[0], glm::vec3(-0.251434f, 0.484739f, 0.032068f), 35.0f, top_left_left_left);
    auto* topLeftLeftRightPhaser = NEW PhaserBeam(_this, map, top_left_left_right[0], glm::vec3(-0.251434f, 0.484739f, 0.032068f), 35.0f, top_left_left_right);

    auto* topRightLeftPhaser = NEW PhaserBeam(_this, map, top_right_left[0], glm::vec3(0.188111f, 0.484739f, -0.169887f), 35.0f, top_right_left);
    auto* topRightRightPhaser = NEW PhaserBeam(_this, map, top_right_right[0], glm::vec3(0.188111f, 0.484739f, -0.169887f), 35.0f, top_right_right);

    auto* topRightRightLeftPhaser = NEW PhaserBeam(_this, map, top_right_right_left[0], glm::vec3(0.251434f, 0.484739f, 0.032068f), 35.0f, top_right_right_left);
    auto* topRightRightRightPhaser = NEW PhaserBeam(_this, map, top_right_right_right[0], glm::vec3(0.251434f, 0.484739f, 0.032068f), 35.0f, top_right_right_right);

    weapons.addPrimaryWeaponBeam(*topCentralPhaserLeft, true);
    weapons.addPrimaryWeaponBeam(*topCentralPhaserRight, true);
    weapons.addPrimaryWeaponBeam(*topLeftLeftPhaser);
    weapons.addPrimaryWeaponBeam(*topLeftRightPhaser);
    weapons.addPrimaryWeaponBeam(*topLeftLeftLeftPhaser);
    weapons.addPrimaryWeaponBeam(*topLeftLeftRightPhaser);
    weapons.addPrimaryWeaponBeam(*topRightLeftPhaser);
    weapons.addPrimaryWeaponBeam(*topRightRightPhaser);
    weapons.addPrimaryWeaponBeam(*topRightRightLeftPhaser);
    weapons.addPrimaryWeaponBeam(*topRightRightRightPhaser);

    //aft left phaser
    vector<glm::vec3> aft_left{ glm::vec3(-0.083414f,-0.211673f, 2.62571f) };
    //aft right phaser
    vector<glm::vec3> aft_right{ glm::vec3(0.083414f,-0.211673f, 2.62571f) };
    auto* aft_left_phaser = NEW PhaserBeam(_this, map, aft_left[0], glm::vec3(-0.101758f, 0.0, 0.308358f), 30.0f, aft_left);
    auto* aft_right_phaser = NEW PhaserBeam(_this, map, aft_right[0], glm::vec3(0.101758f, 0.0, 0.308358f), 30.0f, aft_right);
    weapons.addPrimaryWeaponBeam(*aft_left_phaser);
    weapons.addPrimaryWeaponBeam(*aft_right_phaser);

    //bottom saucer phasers

    //bottom front phaser
    vector<glm::vec3> bottom_front_left{ glm::vec3(0.018227f,0.017345f, -2.51775f) };
    vector<glm::vec3> bottom_front_right{ glm::vec3(-0.018227f,0.017345f, -2.51775f) };
    //bottom left phaser
    vector<glm::vec3> bottom_left_left{ glm::vec3(0.518238f,0.017345f, -2.2097f) };
    vector<glm::vec3> bottom_left_right{ glm::vec3(0.49965f,0.017345f, -2.24585f) };
    //bottom left2 phaser
    vector<glm::vec3> bottom_left2_left{ glm::vec3(0.499424f,0.017345f, -1.62272f) };
    vector<glm::vec3> bottom_left2_right{ glm::vec3(0.519743f,0.017345f, -1.65857f) };
    //bottom right
    vector<glm::vec3> bottom_right_left{ glm::vec3(-0.518238f,0.017345f, -2.2097f) };
    vector<glm::vec3> bottom_right_right{ glm::vec3(-0.49965f,0.017345f, -2.24585f) };
    //bottom right2
    vector<glm::vec3> bottom_right2_left{ glm::vec3(-0.499424f,0.017345f, -1.62272f) };
    vector<glm::vec3> bottom_right2_right{ glm::vec3(-0.519743f,0.017345f, -1.65857f) };


    auto* btmCentralPhaserLeft = NEW PhaserBeam(_this, map, bottom_front_left[0], glm::vec3(0.0f, -0.407625f, -0.162517f), 35.0f, bottom_front_left);
    auto* btmCentralPhaserRight = NEW PhaserBeam(_this, map, bottom_front_right[0], glm::vec3(0.0f, -0.407625f, -0.162517f), 35.0f, bottom_front_right);

    auto* btmLeftLeftPhaser = NEW PhaserBeam(_this, map, bottom_left_left[0], glm::vec3(0.140312f, -0.407625f, -0.082001f), 35.0f, bottom_left_left);
    auto* btmLeftRightPhaser = NEW PhaserBeam(_this, map, bottom_left_right[0], glm::vec3(0.140312f, -0.407625f, -0.082001f), 35.0f, bottom_left_right);

    auto* btmLeft2LeftPhaser = NEW PhaserBeam(_this, map, bottom_left2_left[0], glm::vec3(0.141667f, -0.407625f, 0.079637f), 35.0f, bottom_left2_left);
    auto* btmLeft2RightPhaser = NEW PhaserBeam(_this, map, bottom_left2_right[0], glm::vec3(0.141667f, -0.407625f, 0.079637f), 35.0f, bottom_left2_right);

    auto* btmRightLeftPhaser = NEW PhaserBeam(_this, map, bottom_right_left[0], glm::vec3(-0.140312f, -0.407625f, -0.082001f), 35.0f, bottom_right_left);
    auto* btmRightRightPhaser = NEW PhaserBeam(_this, map, bottom_right_right[0], glm::vec3(-0.140312f, -0.407625f, -0.082001f), 35.0f, bottom_right_right);

    auto* topRight2LeftPhaser = NEW PhaserBeam(_this, map, bottom_right2_left[0], glm::vec3(-0.141667f, -0.407625f, 0.079637f), 35.0f, bottom_right2_left);
    auto* topRight2RightPhaser = NEW PhaserBeam(_this, map, bottom_right2_right[0], glm::vec3(-0.141667f, -0.407625f, 0.079637f), 35.0f, bottom_right2_right);

    weapons.addPrimaryWeaponBeam(*btmCentralPhaserLeft, true);
    weapons.addPrimaryWeaponBeam(*btmCentralPhaserRight, true);
    weapons.addPrimaryWeaponBeam(*btmLeftLeftPhaser);
    weapons.addPrimaryWeaponBeam(*btmLeftRightPhaser);
    weapons.addPrimaryWeaponBeam(*btmLeft2LeftPhaser);
    weapons.addPrimaryWeaponBeam(*btmLeft2RightPhaser);
    weapons.addPrimaryWeaponBeam(*btmRightLeftPhaser);
    weapons.addPrimaryWeaponBeam(*btmRightRightPhaser);
    weapons.addPrimaryWeaponBeam(*topRight2LeftPhaser);
    weapons.addPrimaryWeaponBeam(*topRight2RightPhaser);

    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, -0.306522f, -0.368403f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Excelsior::~Excelsior() {

}
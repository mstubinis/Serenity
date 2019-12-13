#include "FedDefPlatform.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/cannons/PulsePhaser.h"
#include "../../weapons/torpedos/PhotonTorpedo.h"
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
#include "../../ai/AIStationaryNPC.h"
#include "../Ships.h"

#include <iostream>

using namespace std;

constexpr auto CLASS = "Federation Defense Platform";

FedDefPlatform::FedDefPlatform(Team& team, Client& client, Map& map, const std::string& name, glm::vec3 position, glm::vec3 scale)
:Ship(team, client, CLASS, map, AIType::AI_Stationary, name, position, scale, CollisionType::TriangleShapeStatic)
{
    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = nullptr; //no pitch thrusters
        else if (i == 2)   system = nullptr; //no yaw thrusters
        else if (i == 3)   system = nullptr; //no roll thrusters
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = NEW ShipSystemShields(_this, map, 9000.0f, 9000.0f, 9000.0f, 9000.0f, 9000.0f, 9000.0f);
        else if (i == 6)   system = nullptr; //no main thrusters
        else if (i == 7)   system = nullptr; //no warp drive
        else if (i == 8)   system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)   system = NEW ShipSystemWeapons(_this);
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 19000.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* torp_1  = NEW PhotonTorpedo(_this, map, glm::vec3(-1.34187f, 0.0f, -0.107516f), glm::vec3(-1, 0, 0), 45.0f);
    auto* torp_2  = NEW PhotonTorpedo(_this, map, glm::vec3(-1.34187f, 0.0f, 0.107516f), glm::vec3(-1, 0, 0), 45.0f);
    auto* torp_3  = NEW PhotonTorpedo(_this, map, glm::vec3(1.34187f, 0.0f, -0.107516f), glm::vec3(1, 0, 0), 45.0f);
    auto* torp_4  = NEW PhotonTorpedo(_this, map, glm::vec3(1.34187f, 0.0f, 0.107516f), glm::vec3(1, 0, 0), 45.0f);

    auto* torp_5  = NEW PhotonTorpedo(_this, map, glm::vec3(0.112896f, 0.0f, -1.34187f), glm::vec3(0, 0, -1), 45.0f);
    auto* torp_6  = NEW PhotonTorpedo(_this, map, glm::vec3(-0.112896f, 0.0f,-1.34187f), glm::vec3(0, 0, -1), 45.0f);
    auto* torp_7  = NEW PhotonTorpedo(_this, map, glm::vec3(0.112896f, 0.0f, 1.34187f), glm::vec3(0, 0, 1), 45.0f);
    auto* torp_8  = NEW PhotonTorpedo(_this, map, glm::vec3(-0.112896f, 0.0f, 1.34187f), glm::vec3(0, 0, 1), 45.0f);

    auto* torp_9  = NEW PhotonTorpedo(_this, map, glm::vec3(0.113388f, -1.337f, 0.0f), glm::vec3(0, -1, 0), 45.0f);
    auto* torp_10 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.113388f, -1.337f, 0.0f), glm::vec3(0, -1, 0), 45.0f);
    auto* torp_11 = NEW PhotonTorpedo(_this, map, glm::vec3(0.113388f, 1.337f, 0.0f), glm::vec3(0, 1, 0), 45.0f);
    auto* torp_12 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.113388f, 1.337f, 0.0f), glm::vec3(0, 1, 0), 45.0f);

    weapons.addSecondaryWeaponTorpedo(*torp_1,true);
    weapons.addSecondaryWeaponTorpedo(*torp_2,true);
    weapons.addSecondaryWeaponTorpedo(*torp_3);
    weapons.addSecondaryWeaponTorpedo(*torp_4);
    weapons.addSecondaryWeaponTorpedo(*torp_5);
    weapons.addSecondaryWeaponTorpedo(*torp_6);
    weapons.addSecondaryWeaponTorpedo(*torp_7);
    weapons.addSecondaryWeaponTorpedo(*torp_8);
    weapons.addSecondaryWeaponTorpedo(*torp_9);
    weapons.addSecondaryWeaponTorpedo(*torp_10);
    weapons.addSecondaryWeaponTorpedo(*torp_11);
    weapons.addSecondaryWeaponTorpedo(*torp_12);

    vector<glm::vec3> phaser_1_pts{
        glm::vec3(-1.36985f, 0.0f, 0.0f),
        glm::vec3(-1.26459f, 0.0f, 0.53017f),
        glm::vec3(-0.971708f, 0.0f, 0.975349f),
        glm::vec3(0.484354f, 0.180227f, 1.28697f),
        glm::vec3(0.0f, 0.0f, 1.37011f),
    };
    auto* phaser_1 = NEW PhaserBeam(_this, map, phaser_1_pts[2], glm::vec3(-0.971708f, 0.0f, 0.975349f), 45.0f, phaser_1_pts);
    vector<glm::vec3> phaser_2_pts{
        glm::vec3(0.0f, 0.0f, -1.36985f),
        glm::vec3(-0.53017f, 0.0f, -1.26459f),
        glm::vec3(-0.975349f, 0.0f, -0.971708f),
        glm::vec3(-1.28697f, 0.0f, -0.484354f),
        glm::vec3(-1.37011f, 0.0f, 0.0f),
    };
    auto* phaser_2 = NEW PhaserBeam(_this, map, phaser_2_pts[2], glm::vec3(-0.975349f, 0.0f, -0.971708f), 45.0f, phaser_2_pts);
    vector<glm::vec3> phaser_3_pts{
        glm::vec3(1.36985f, 0.0f, 0.0f),
        glm::vec3(1.26459f, 0.0f, -0.53017f),
        glm::vec3(0.971708f, 0.0f, -0.975349f),
        glm::vec3(0.484354f, 0.0f, -1.28697f),
        glm::vec3(0.0f, 0.0f, -1.37011f),
    };
    auto* phaser_3 = NEW PhaserBeam(_this, map, phaser_3_pts[2], glm::vec3(0.971708f, 0.0f, -0.975349f), 45.0f, phaser_3_pts);
    vector<glm::vec3> phaser_4_pts{
        glm::vec3(0.0f, 0.0f, 1.36985f),
        glm::vec3(0.53017f, 0.0f, 1.26459f),
        glm::vec3(0.975349f, 0.0f, 0.971708f),
        glm::vec3(1.28697f, 0.0f, 0.484354f),
        glm::vec3(1.37011f, 0.0f, 0.0f),
    };
    auto* phaser_4 = NEW PhaserBeam(_this, map, phaser_4_pts[2], glm::vec3(0.975349f, 0.0f, 0.971708f), 45.0f, phaser_4_pts);
    weapons.addPrimaryWeaponBeam(*phaser_1);
    weapons.addPrimaryWeaponBeam(*phaser_2);
    weapons.addPrimaryWeaponBeam(*phaser_3);
    weapons.addPrimaryWeaponBeam(*phaser_4);

    vector<glm::vec3> phaser_5_pts{
        glm::vec3(-1.37011f, 0.0f, 0.0f),
        glm::vec3(-1.28697f, 0.484354f, 0.0f),
        glm::vec3(-0.975349f, 0.971708f, 0.0f),
        glm::vec3(-0.53017f, 1.26459f, 0.0f),
        glm::vec3(0.0f, 1.36985f, 0.0f),
    };
    auto* phaser_5 = NEW PhaserBeam(_this, map, phaser_5_pts[2], glm::vec3(-0.975349f, 0.971708f, 0.0f), 45.0f, phaser_5_pts);
    vector<glm::vec3> phaser_6_pts{
        glm::vec3(0.0f, 1.37011f, 0.0f),
        glm::vec3(0.484354f, 1.28697f, 0.0f),
        glm::vec3(0.971708f, 0.975349f, 0.0f),
        glm::vec3(1.26459f, 0.53017f, 0.0f),
        glm::vec3(1.36985f, 0.0f, 0.0f),
    };
    auto* phaser_6 = NEW PhaserBeam(_this, map, phaser_6_pts[2], glm::vec3(0.971708f, 0.975349f, 0.0f), 45.0f, phaser_6_pts);
    vector<glm::vec3> phaser_7_pts{
        glm::vec3(1.37011f, 0.0f, 0.0f),
        glm::vec3(1.28697f, -0.484354f, 0.0f),
        glm::vec3(0.975349f, -0.971708f, 0.0f),
        glm::vec3(0.530171f, -1.26459f, 0.0f),
        glm::vec3(0.0f, -1.36985f, 0.0f),
    };
    auto* phaser_7 = NEW PhaserBeam(_this, map, phaser_7_pts[2], glm::vec3(0.975349f, -0.971708f, 0.0f), 45.0f, phaser_7_pts);
    vector<glm::vec3> phaser_8_pts{
        glm::vec3(-1.36985f, 0.0f, 0.0f),
        glm::vec3(-1.26459f, -0.530171f, 0.0f),
        glm::vec3(-0.971708f, -0.975349f, 0.0f),
        glm::vec3(-0.484354f, -1.28697f, 0.0f),
        glm::vec3(0.0f, -1.37011f, 0.0f),
    };
    auto* phaser_8 = NEW PhaserBeam(_this, map, phaser_8_pts[2], glm::vec3(-0.971708f, -0.975349f, 0.0f), 45.0f, phaser_8_pts);
    weapons.addPrimaryWeaponBeam(*phaser_5);
    weapons.addPrimaryWeaponBeam(*phaser_6);
    weapons.addPrimaryWeaponBeam(*phaser_7);
    weapons.addPrimaryWeaponBeam(*phaser_8);






    vector<glm::vec3> phaser_9_pts{
        glm::vec3(0.0f, 0.0f, -1.37085f),
        glm::vec3(0.0f, 0.506409f, -1.27503f),
        glm::vec3(0.0f, 0.956163f, -0.974392f),
        glm::vec3(0.0f, 1.25139f, -0.5503f),
        glm::vec3(0.0f, 1.37085f, 0.0f),
    };
    auto* phaser_9 = NEW PhaserBeam(_this, map, phaser_9_pts[2], glm::vec3(0.0f, 0.956163f, -0.974392f), 45.0f, phaser_9_pts);
    vector<glm::vec3> phaser_10_pts{
        glm::vec3(0.0f, 1.37085f, 0.0f),
        glm::vec3(0.0f, 1.27503f, 0.506409f),
        glm::vec3(0.0f, 0.974392f, 0.956163f),
        glm::vec3(0.0f, 0.5503f, 1.25139f),
        glm::vec3(0.0f, 0.0f, 1.37085f),
    };
    auto* phaser_10 = NEW PhaserBeam(_this, map, phaser_10_pts[2], glm::vec3(0.0f, 0.974392f, 0.956163f), 45.0f, phaser_10_pts);
    vector<glm::vec3> phaser_11_pts{
        glm::vec3(0.0f, 0.0f, 1.37085f),
        glm::vec3(0.0f, -0.50641f, 1.27503f),
        glm::vec3(0.0f, -0.956163f, 0.974392f),
        glm::vec3(0.0f, -1.25139f, 0.5503f),
        glm::vec3(0.0f, -1.37085f, 0.0f),
    };
    auto* phaser_11 = NEW PhaserBeam(_this, map, phaser_11_pts[2], glm::vec3(0.0f, -0.956163f, 0.974392f), 45.0f, phaser_11_pts);
    vector<glm::vec3> phaser_12_pts{
        glm::vec3(0.0f, 0.0f, -1.37085f),
        glm::vec3(0.0f, -0.5503f, -1.25139f),
        glm::vec3(0.0f, -0.974392f, -0.956163f),
        glm::vec3(0.0f, -1.27503f, -0.50641f),
        glm::vec3(0.0f, -1.37085f, 0.0f),
    };
    auto* phaser_12 = NEW PhaserBeam(_this, map, phaser_12_pts[2], glm::vec3(0.0f, -0.974392f, -0.956163f), 45.0f, phaser_12_pts);
    weapons.addPrimaryWeaponBeam(*phaser_9);
    weapons.addPrimaryWeaponBeam(*phaser_10);
    weapons.addPrimaryWeaponBeam(*phaser_11);
    weapons.addPrimaryWeaponBeam(*phaser_12);


    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 0.0f, 0.0f),
        };
    }


    internal_finialize_init(AIType::AI_Stationary);
}
FedDefPlatform::~FedDefPlatform() {

}
#include "FedDefPlatform.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PulsePhaser.h"
#include "../../weapons/PhotonTorpedo.h"
#include "../../weapons/PhaserBeam.h"

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

#include <iostream>

using namespace std;

FedDefPlatform::FedDefPlatform(Team& team, Client& client, Map& map, const std::string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team, client, "Federation Defense Platform", map, AIType::AI_Stationary, name, position, scale, collisionType)
{
    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = nullptr; //no pitch thrusters
        else if (i == 2)   system = nullptr; //no yaw thrusters
        else if (i == 3)   system = nullptr; //no roll thrusters
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = new ShipSystemShields(_this, map, 9000.0f, 9000.0f, 9000.0f, 9000.0f, 9000.0f, 9000.0f);
        else if (i == 6)   system = nullptr; //no main thrusters
        else if (i == 7)   system = nullptr; //no warp drive
        else if (i == 8)   system = new ShipSystemSensors(_this, map);
        else if (i == 9)   system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 19000.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* torp_1  = new PhotonTorpedo(_this, map, glm::vec3(-1.34187f, 0.0f, -0.107516f), glm::vec3(-1, 0, 0), 45.0f);
    auto* torp_2  = new PhotonTorpedo(_this, map, glm::vec3(-1.34187f, 0.0f, 0.107516f), glm::vec3(-1, 0, 0), 45.0f);
    auto* torp_3  = new PhotonTorpedo(_this, map, glm::vec3(1.34187f, 0.0f, -0.107516f), glm::vec3(1, 0, 0), 45.0f);
    auto* torp_4  = new PhotonTorpedo(_this, map, glm::vec3(1.34187f, 0.0f, 0.107516f), glm::vec3(1, 0, 0), 45.0f);

    auto* torp_5  = new PhotonTorpedo(_this, map, glm::vec3(0.112896f, 0.0f, -1.34187f), glm::vec3(0, 0, -1), 45.0f);
    auto* torp_6  = new PhotonTorpedo(_this, map, glm::vec3(-0.112896f, 0.0f,-1.34187f), glm::vec3(0, 0, -1), 45.0f);
    auto* torp_7  = new PhotonTorpedo(_this, map, glm::vec3(0.112896f, 0.0f, 1.34187f), glm::vec3(0, 0, 1), 45.0f);
    auto* torp_8  = new PhotonTorpedo(_this, map, glm::vec3(-0.112896f, 0.0f, 1.34187f), glm::vec3(0, 0, 1), 45.0f);

    auto* torp_9  = new PhotonTorpedo(_this, map, glm::vec3(0.113388f, -1.337f, 0.0f), glm::vec3(0, -1, 0), 45.0f);
    auto* torp_10 = new PhotonTorpedo(_this, map, glm::vec3(-0.113388f, -1.337f, 0.0f), glm::vec3(0, -1, 0), 45.0f);
    auto* torp_11 = new PhotonTorpedo(_this, map, glm::vec3(0.113388f, 1.337f, 0.0f), glm::vec3(0, 1, 0), 45.0f);
    auto* torp_12 = new PhotonTorpedo(_this, map, glm::vec3(-0.113388f, 1.337f, 0.0f), glm::vec3(0, 1, 0), 45.0f);

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

    internal_finialize_init(AIType::AI_Stationary);
}
FedDefPlatform::~FedDefPlatform() {

}
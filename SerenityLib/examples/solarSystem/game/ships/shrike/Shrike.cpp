#include "Shrike.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PlasmaCannon.h"
#include "../../weapons/PlasmaBeam.h"
#include "../../weapons/PlasmaTorpedo.h"


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

using namespace std;

Shrike::Shrike(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client, ResourceManifest::ShrikeMesh, ResourceManifest::ShrikeMaterial, "Shrike", map, player, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = new ShipSystemShields(_this, map, 8000.0f, 8000.0f, 8000.0f, 8000.0f, 12000.0f, 12000.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 7500.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTop     = new PlasmaCannon(_this, map, glm::vec3(-0.934207f, 0.02951f, -0.224055f), glm::vec3(0.009f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);
    auto* leftBottom  = new PlasmaCannon(_this, map, glm::vec3(-0.308448f, 0.032778f, -0.819245f), glm::vec3(0.0008f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);
    auto* rightBottom = new PlasmaCannon(_this, map, glm::vec3(0.308448f, 0.032778f, -0.819245f), glm::vec3(-0.0008f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);
    auto* rightTop    = new PlasmaCannon(_this, map, glm::vec3(0.934207f, 0.02951f, -0.224055f), glm::vec3(-0.009f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);

    weapons.addPrimaryWeaponCannon(*leftTop);
    weapons.addPrimaryWeaponCannon(*leftBottom);
    weapons.addPrimaryWeaponCannon(*rightBottom);
    weapons.addPrimaryWeaponCannon(*rightTop);

    auto* frontTorp = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.049485f, -1.23634f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* frontTorp1 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.049485f, -1.23634f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* aftTorp = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.055816f, 1.46661f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addSecondaryWeaponTorpedo(*frontTorp);
    weapons.addSecondaryWeaponTorpedo(*frontTorp1);
    weapons.addSecondaryWeaponTorpedo(*aftTorp);

    vector<glm::vec3> beam_pt{ glm::vec3(0.0f, -0.049803f, -1.26533f) };
    auto* frontBeam = new PlasmaBeam(_this, map, beam_pt[0], glm::vec3(0, 0, -1), 30.0f, beam_pt);
    weapons.addPrimaryWeaponBeam(*frontBeam);

}
Shrike::~Shrike() {

}
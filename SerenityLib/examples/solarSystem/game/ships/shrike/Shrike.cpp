#include "Shrike.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PlasmaCannon.h"
#include "../../weapons/DisruptorCannon.h"

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

    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(*this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
        else if (i == 2)  system = new ShipSystemYawThrusters(*this);
        else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(*this);
        else if (i == 5)  system = new ShipSystemShields(*this, map, 8000);
        else if (i == 6)  system = new ShipSystemMainThrusters(*this);
        else if (i == 7)  system = new ShipSystemWarpDrive(*this);
        else if (i == 8)  system = new ShipSystemSensors(*this, map);
        else if (i == 9)  system = new ShipSystemWeapons(*this);
        else if (i == 10)  system = new ShipSystemHull(*this, map, 7500);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTop     = new PlasmaCannon(*this, map, glm::vec3(-0.934207f, 0.02951f, -0.224055f), glm::vec3(0.009f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);
    auto* leftBottom  = new PlasmaCannon(*this, map, glm::vec3(-0.308448f, 0.032778f, -0.819245f), glm::vec3(0.0008f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);
    auto* rightBottom = new PlasmaCannon(*this, map, glm::vec3(0.308448f, 0.032778f, -0.819245f), glm::vec3(-0.0008f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);
    auto* rightTop    = new PlasmaCannon(*this, map, glm::vec3(0.934207f, 0.02951f, -0.224055f), glm::vec3(-0.009f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 75.0f);

    weapons.addPrimaryWeaponCannon(*leftTop);
    weapons.addPrimaryWeaponCannon(*leftBottom);
    weapons.addPrimaryWeaponCannon(*rightBottom);
    weapons.addPrimaryWeaponCannon(*rightTop);

}
Shrike::~Shrike() {

}
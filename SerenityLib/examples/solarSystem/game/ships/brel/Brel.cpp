#include "Brel.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
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

Brel::Brel(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client, ResourceManifest::BrelMesh, ResourceManifest::BrelMaterial, "Brel", map,player, name, position, scale, collisionType, glm::vec3(0.0f, 0.311455f, 0.397761f), glm::vec3(0.0f,0.7f,0.7f)) {

    //blender 3d to game 3d: switch y and z, then negate z

    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(*this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
        else if (i == 2)  system = new ShipSystemYawThrusters(*this);
        else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(*this);
        else if (i == 5)  system = new ShipSystemShields(*this, map, 7700.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(*this);
        else if (i == 7)  system = new ShipSystemWarpDrive(*this);
        else if (i == 8)  system = new ShipSystemSensors(*this, map);
        else if (i == 9)  system = new ShipSystemWeapons(*this);
        else if (i == 10)  system = new ShipSystemHull(*this, map, 7100.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //blender 3d to game 3d: switch y and z, then negate z
    auto* leftTop = new DisruptorCannon(*this, map, glm::vec3(-1.01261f, -0.315514f, -0.550506f), glm::vec3(0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);
    auto* leftBottom = new DisruptorCannon(*this, map, glm::vec3(-1.01515f, -0.338814f, -0.544875f), glm::vec3(0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);
    auto* rightBottom = new DisruptorCannon(*this, map, glm::vec3(1.01515f, -0.338814f, -0.544875f), glm::vec3(-0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);
    auto* rightTop = new DisruptorCannon(*this, map, glm::vec3(1.01261f, -0.315514f, -0.550506f), glm::vec3(-0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);

    weapons.addPrimaryWeaponCannon(*leftTop);
    weapons.addPrimaryWeaponCannon(*leftBottom);
    weapons.addPrimaryWeaponCannon(*rightBottom);
    weapons.addPrimaryWeaponCannon(*rightTop);
}
Brel::~Brel() {

}
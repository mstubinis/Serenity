#include "Miranda.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PulsePhaser.h"

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

Miranda::Miranda(Client& client, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType, Map* map)
:Ship(client, ResourceManifest::MirandaMesh, ResourceManifest::MirandaMaterial, "Miranda", player, name, position, scale, collisionType, map) {

    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(*this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
        else if (i == 2)  system = new ShipSystemYawThrusters(*this);
        else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(*this);  //TODO: remove cloaking device
        else if (i == 5)  system = new ShipSystemShields(*this, map, 8500);
        else if (i == 6)  system = new ShipSystemMainThrusters(*this);
        else if (i == 7)  system = new ShipSystemWarpDrive(*this);
        else if (i == 8)  system = new ShipSystemSensors(*this);
        else if (i == 9)  system = new ShipSystemWeapons(*this);
        else if (i == 10)  system = new ShipSystemHull(*this, 8000);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

}
Miranda::~Miranda() {

}
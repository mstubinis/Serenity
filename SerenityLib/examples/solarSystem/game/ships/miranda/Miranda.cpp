#include "Miranda.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PhotonTorpedoOld.h"

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

Miranda::Miranda(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client, ResourceManifest::MirandaMesh, ResourceManifest::MirandaMaterial, "Miranda", map, player, name, position, scale, collisionType, glm::vec3(0.0f, 0.107337f, -0.272124f)) {

    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(*this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
        else if (i == 2)  system = new ShipSystemYawThrusters(*this);
        else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = nullptr; //no cloaking device
        else if (i == 5)  system = new ShipSystemShields(*this, map, 8500);
        else if (i == 6)  system = new ShipSystemMainThrusters(*this);
        else if (i == 7)  system = new ShipSystemWarpDrive(*this);
        else if (i == 8)  system = new ShipSystemSensors(*this, map);
        else if (i == 9)  system = new ShipSystemWeapons(*this);
        else if (i == 10)  system = new ShipSystemHull(*this, map, 8000);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTorp = new PhotonTorpedoOld(*this, map, glm::vec3(-0.03427f, 0.310924f, 0.07898f), glm::vec3(0, 0, -1), 15.0f);
    auto* rightTorp = new PhotonTorpedoOld(*this, map, glm::vec3(0.03427f, 0.310924f, 0.07898f), glm::vec3(0, 0, -1), 15.0f);
    auto* aftTorpLeft = new PhotonTorpedoOld(*this, map, glm::vec3(-0.052569f, 0.306842f, 0.405438f), glm::vec3(0, 0, 1), 15.0f);
    auto* aftTorpRight = new PhotonTorpedoOld(*this, map, glm::vec3(0.052569f, 0.306842f, 0.405438f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addSecondaryWeaponTorpedo(*leftTorp);
    weapons.addSecondaryWeaponTorpedo(*rightTorp);
    weapons.addSecondaryWeaponTorpedo(*aftTorpLeft);
    weapons.addSecondaryWeaponTorpedo(*aftTorpRight);
}
Miranda::~Miranda() {

}
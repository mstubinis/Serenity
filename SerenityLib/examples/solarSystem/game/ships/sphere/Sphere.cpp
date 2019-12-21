#include "Sphere.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"

#include "../../weapons/beams/BorgBeam.h"
#include "../../weapons/beams/BorgCuttingBeam.h"
#include "../../weapons/torpedos/BorgTorpedo.h"

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

constexpr auto CLASS = "Sphere";

using namespace std;

Sphere::Sphere(Scene& scene, glm::vec3 position, glm::vec3 scale)
    :Ship(CLASS, scene, position, scale) {

}

Sphere::Sphere(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team, client, CLASS, map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = NEW ShipSystemPitchThrusters(_this);
        else if (i == 2)   system = NEW ShipSystemYawThrusters(_this);
        else if (i == 3)   system = NEW ShipSystemRollThrusters(_this);
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = nullptr; //borg dont use shields
        else if (i == 6)   system = NEW ShipSystemMainThrusters(_this);
        else if (i == 7)   system = NEW ShipSystemWarpDrive(_this);
        else if (i == 8)   system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)   system = NEW ShipSystemWeapons(_this);
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 85500.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f,0.0f, 0.0f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), weapons);
    m_AI->installThreatTable(map);
}
Sphere::~Sphere() {

}

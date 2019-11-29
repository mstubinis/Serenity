#include "FedStarbaseMushroom.h"
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
#include "../Ships.h"

#include <iostream>

using namespace std;

constexpr auto CLASS = "Federation Starbase Mushroom";

FedStarbaseMushroom::FedStarbaseMushroom(Team& team, Client& client, Map& map, const std::string& name, glm::vec3 position, glm::vec3 scale)
:Ship(team, client, CLASS, map, AIType::AI_Stationary, name, position, scale, CollisionType::TriangleShapeStatic){
    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = nullptr; //no pitch thrusters
        else if (i == 2)   system = nullptr; //no yaw thrusters
        else if (i == 3)   system = nullptr; //no roll thrusters
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = new ShipSystemShields(_this, map, 160000.0f, 160000.0f, 160000.0f, 160000.0f, 160000.0f, 160000.0f);
        else if (i == 6)   system = nullptr; //no main thrusters
        else if (i == 7)   system = nullptr; //no warp drive
        else if (i == 8)   system = new ShipSystemSensors(_this, map);
        else if (i == 9)   system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 200000.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 0.0f, 0.0f),
        };
    }


    internal_finialize_init(AIType::AI_Stationary);
}
FedStarbaseMushroom::~FedStarbaseMushroom() {

}
#include "Norway.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/torpedos/PhotonTorpedo.h"
#include "../../weapons/beams/PhaserBeam.h"
#include "../../weapons/cannons/PulsePhaser.h"

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

constexpr auto CLASS = "Norway";

using namespace std;

Norway::Norway(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS, map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)   system = new ShipSystemYawThrusters(_this);
        else if (i == 3)   system = new ShipSystemRollThrusters(_this);
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = new ShipSystemShields(_this, map, 8500.0f, 8500.0f, 8500.0f, 8500.0f, 11500.0f, 11500.0f);
        else if (i == 6)   system = new ShipSystemMainThrusters(_this);
        else if (i == 7)   system = new ShipSystemWarpDrive(_this);
        else if (i == 8)   system = new ShipSystemSensors(_this, map);
        else if (i == 9)   system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 17800.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));


    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 0.0f, -1.58693f),
            glm::vec3(0,-0.12653f, -1.68783f),
            glm::vec3(0,-0.086775f, -1.39145f),
            glm::vec3(0,-0.043406f, -1.04447f),
            glm::vec3(0,0.010806f, -0.733638f),
            glm::vec3(0,0.010806f, -0.477052f),
            glm::vec3(0,0.061423f, -0.148036f),
            glm::vec3(-0.943021f,-0.01354f,0.295371f),
            glm::vec3(0.943021f,-0.01354f, 0.295371f),
            glm::vec3(-0.943021f,-0.01354f,0.797967f),
            glm::vec3(0.943021f,-0.01354f, 0.797967f),
            glm::vec3(-0.943021f,-0.01354f,1.3579f),
            glm::vec3(0.943021f,-0.01354f, 1.3579f),
            glm::vec3(-0.74871f,0.058747f,1.14439f),
            glm::vec3(0.74871f,0.058747f, 1.14439f),
            glm::vec3(-0.432982f,0.142455f,0.865092f),
            glm::vec3(0.432982f,0.142455f, 0.865092f),
            glm::vec3(-0.432982f,0.142455f,1.271f),
            glm::vec3(0.432982f,0.142455f, 1.271f),
            glm::vec3(-0.645729f,-0.117819f,-0.682427f),
            glm::vec3(0.645729f,-0.117819f, -0.682427f),
            glm::vec3(-0.467915f,-0.117819f,-0.986012f),
            glm::vec3(0.467915f,-0.117819f, -0.986012f),
            glm::vec3(-0.318995f,-0.117819f,-1.37057f),
            glm::vec3(0.318995f,-0.117819f, -1.37057f),
            glm::vec3(-0.181716f,-0.013666f,-0.943106f),
            glm::vec3(0.181716f,-0.013666f, -0.943106f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Norway::~Norway() {

}

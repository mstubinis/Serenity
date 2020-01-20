#include "Shrike.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/cannons/PlasmaCannon.h"
#include "../../weapons/beams/PlasmaBeam.h"
#include "../../weapons/torpedos/PlasmaTorpedo.h"


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

constexpr auto CLASS = "Shrike";

using namespace std;

Shrike::Shrike(Scene& scene, glm::vec3 position, glm::vec3 scale)
    :Ship(CLASS, scene, position, scale) {

}

Shrike::Shrike(AIType::Type& ai_type, Team& team, Client& client, Map& map,  const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS, map, ai_type, name, position, scale, collisionType, glm::vec3(0.0f, 0.1f, 0.0f)) {

    m_Perks         = Shrike::Perks::None;
    m_UnlockedPerks = Shrike::Perks::None;

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if      (i == 0)  system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = NEW ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = NEW ShipSystemYawThrusters(_this);
        else if (i == 3)  system = NEW ShipSystemRollThrusters(_this);
        else if (i == 4)  system = NEW ShipSystemCloakingDevice(_this, (m_UnlockedPerks & Shrike::Perks::TalShiarCloak) ? 1.39f : 1.0f);
        else if (i == 5)  system = NEW ShipSystemShields(_this, map, 8000.0f, 8000.0f, 8000.0f, 8000.0f, 12000.0f, 12000.0f, glm::vec3(0.0f), glm::vec3(1.15f, 2.1f, 1.15f));
        else if (i == 6)  system = NEW ShipSystemMainThrusters(_this);
        else if (i == 7)  system = NEW ShipSystemWarpDrive(_this);
        else if (i == 8)  system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)  system = NEW ShipSystemWeapons(_this);
        else if (i == 10) system = NEW ShipSystemHull(_this, map, 16500.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    const float base_cannon_dmg = 250.0f;
    const float base_beam_dmg = 750.0f;
    const float base_torpedo_dmg = 6650.0f;

    auto* leftTop     = NEW PlasmaCannon(_this, map, glm::vec3(-0.934207f, 0.02951f, -0.224055f), glm::vec3(0.009f, 0, -1), 17.0f, 6, base_cannon_dmg, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);
    auto* leftBottom  = NEW PlasmaCannon(_this, map, glm::vec3(-0.308448f, 0.032778f, -0.819245f), glm::vec3(0.0008f, 0, -1), 17.0f, 6, base_cannon_dmg, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);
    auto* rightBottom = NEW PlasmaCannon(_this, map, glm::vec3(0.308448f, 0.032778f, -0.819245f), glm::vec3(-0.0008f, 0, -1), 17.0f, 6, base_cannon_dmg, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);
    auto* rightTop    = NEW PlasmaCannon(_this, map, glm::vec3(0.934207f, 0.02951f, -0.224055f), glm::vec3(-0.009f, 0, -1), 17.0f, 6, base_cannon_dmg, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);

    weapons.addPrimaryWeaponCannon(*leftTop, true);
    weapons.addPrimaryWeaponCannon(*leftBottom, true);
    weapons.addPrimaryWeaponCannon(*rightBottom, true);
    weapons.addPrimaryWeaponCannon(*rightTop, true);

    auto* frontTorp  = NEW PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.049485f, -1.23634f), glm::vec3(0, 0, -1), 15.0f, 1, base_torpedo_dmg);
    auto* frontTorp1 = NEW PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.049485f, -1.23634f), glm::vec3(0, 0, -1), 15.0f, 1, base_torpedo_dmg);
    auto* aftTorp    = NEW PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.055816f, 1.46661f), glm::vec3(0, 0, 1), 15.0f, 1, base_torpedo_dmg);

    weapons.addSecondaryWeaponTorpedo(*frontTorp, true);
    weapons.addSecondaryWeaponTorpedo(*frontTorp1, true);
    weapons.addSecondaryWeaponTorpedo(*aftTorp);

    vector<glm::vec3> beam_pt{ glm::vec3(0.0f, -0.049803f, -1.26533f) };
    auto* frontBeam = NEW PlasmaBeam(_this, map, beam_pt[0], glm::vec3(0, 0, -1), 30.0f, beam_pt, base_beam_dmg);
    weapons.addPrimaryWeaponBeam(*frontBeam, true);


    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0,0, -1.01683f),
            glm::vec3(0,0, -0.695241f),
            glm::vec3(0,0, -0.40262f),
            glm::vec3(0,0, -0.015259f),
            glm::vec3(0,0, 0.290298f),
            glm::vec3(0,0, 0.56271f),
            glm::vec3(0,0, 0.859813f),
            glm::vec3(-1.15097f,-0.039035f, -0.344808f),
            glm::vec3(1.15097f,-0.039035f, -0.344808f),
            glm::vec3(-1.15097f,-0.039035f, 0.125788f),
            glm::vec3(1.15097f,-0.039035f, 0.125788f),
            glm::vec3(-1.15097f,-0.039035f, 0.525935f),
            glm::vec3(1.15097f,-0.039035f, 0.525935),
            glm::vec3(-0.892489f,0.028702f, 0.17479f),
            glm::vec3(0.892489f,0.028702f, 0.17479f),
            glm::vec3(-0.576472f,0.045644f, 0.17479f),
            glm::vec3(0.576472f,0.045644f, 0.17479f),
            glm::vec3(-0.212936f,0.03548f, 0.17479f),
            glm::vec3(0.212936f,0.03548f, 0.17479f),
            glm::vec3(-0.306575f,0.047673f, -0.387041f),
            glm::vec3(0.306575f,0.047673f, -0.387041f),
            glm::vec3(-0.532802f,0.048486f, -0.098031f),
            glm::vec3(0.532802f,0.048486f, -0.098031f),
            glm::vec3(-0.459521f,0.040023f, 0.410263f),
            glm::vec3(0.459521f,0.040023f, 0.410263f),
            glm::vec3(-0.223905f,0.025798f, 0.709812f),
            glm::vec3(0.223905f,0.025798f, 0.709812f),
            glm::vec3(0.0f,-0.091898f, -1.19035f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), weapons);
    m_AI->installThreatTable(map);
}
Shrike::~Shrike() {

}
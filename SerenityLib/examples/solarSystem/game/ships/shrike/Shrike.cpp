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
#include "../../ai/AI.h"

using namespace std;

Shrike::Shrike(AIType::Type& ai_type, Team& team, Client& client, Map& map,  const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, "Shrike", map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = new ShipSystemShields(_this, map, 8000.0f, 8000.0f, 8000.0f, 8000.0f, 12000.0f, 12000.0f, glm::vec3(0.0f), glm::vec3(1.15f, 2.1f, 1.15f));
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 16500.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTop     = new PlasmaCannon(_this, map, glm::vec3(-0.934207f, 0.02951f, -0.224055f), glm::vec3(0.009f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);
    auto* leftBottom  = new PlasmaCannon(_this, map, glm::vec3(-0.308448f, 0.032778f, -0.819245f), glm::vec3(0.0008f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);
    auto* rightBottom = new PlasmaCannon(_this, map, glm::vec3(0.308448f, 0.032778f, -0.819245f), glm::vec3(-0.0008f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);
    auto* rightTop    = new PlasmaCannon(_this, map, glm::vec3(0.934207f, 0.02951f, -0.224055f), glm::vec3(-0.009f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 75.0f);

    weapons.addPrimaryWeaponCannon(*leftTop, true);
    weapons.addPrimaryWeaponCannon(*leftBottom, true);
    weapons.addPrimaryWeaponCannon(*rightBottom, true);
    weapons.addPrimaryWeaponCannon(*rightTop, true);

    auto* frontTorp = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.049485f, -1.23634f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* frontTorp1 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.049485f, -1.23634f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* aftTorp = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.055816f, 1.46661f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addSecondaryWeaponTorpedo(*frontTorp, true);
    weapons.addSecondaryWeaponTorpedo(*frontTorp1, true);
    weapons.addSecondaryWeaponTorpedo(*aftTorp);

    vector<glm::vec3> beam_pt{ glm::vec3(0.0f, -0.049803f, -1.26533f) };
    auto* frontBeam = new PlasmaBeam(_this, map, beam_pt[0], glm::vec3(0, 0, -1), 30.0f, beam_pt, 750.0f);
    weapons.addPrimaryWeaponBeam(*frontBeam, true);


    vector<glm::vec3> hull_target_points = {
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
    addHullTargetPoints(hull_target_points);

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Shrike::~Shrike() {

}
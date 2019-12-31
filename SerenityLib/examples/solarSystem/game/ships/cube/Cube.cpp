#include "Cube.h"
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

constexpr auto CLASS = "Cube";

using namespace std;

Cube::Cube(Scene& scene, glm::vec3 position, glm::vec3 scale)
    :Ship(CLASS, scene, position, scale) {

}

Cube::Cube(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
    : Ship(team, client, CLASS, map, ai_type, name, position, scale, collisionType) {

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
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 5000000.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    const auto cutting_dmg = 2000.0f;
    const auto cutting_impact_rad = 2.5f;
    const auto cutting_impact_time = 1.8f;
    const auto cutting_arc = 55.0f;
    const auto cutting_end_scale = 7.0f;
    const auto cutting_beam_scale = 3.0f;

    //9 fwd cutting beam points
    vector<glm::vec3> fwd_cutting_beams_nine{
        glm::vec3(7.5f,7.5f,-15.075f),
        glm::vec3(0.0f,7.5f,-15.075f),
        glm::vec3(-7.5f,7.5f,-15.075f),
        glm::vec3(7.5f,0.0f,-15.075f),
        glm::vec3(0.0f,0.0f,-15.075f),
        glm::vec3(-7.5f,0.0f,-15.075f),
        glm::vec3(7.5f,-7.5f,-15.075f),
        glm::vec3(0.0f,-7.5f,-15.075f),
        glm::vec3(-7.5f,-7.5f,-15.075f),
    };
    //fwd cutting beams
    for (auto& fwd_beam_pt : fwd_cutting_beams_nine) {
        vector<glm::vec3> windup{ fwd_beam_pt, };
        auto* beam = new BorgCuttingBeam(_this, map, windup[0], glm::vec3(0, 0, -1), cutting_arc, windup, cutting_dmg, 4.0f, 1.5f, cutting_impact_rad, cutting_impact_time, 100.0f, 1, 6.0f, 0, cutting_end_scale, cutting_beam_scale);
        weapons.addPrimaryWeaponBeam(*beam, true);
    }
    //aft cutting beams
    for (auto& fwd_beam_pt : fwd_cutting_beams_nine) {
        vector<glm::vec3> windup{ glm::vec3(fwd_beam_pt.x,fwd_beam_pt.y,-fwd_beam_pt.z), };
        auto* beam = new BorgCuttingBeam(_this, map, windup[0], glm::vec3(0, 0, 1), cutting_arc, windup, cutting_dmg, 4.0f, 1.5f, cutting_impact_rad, cutting_impact_time, 100.0f, 1, 6.0f, 0, cutting_end_scale, cutting_beam_scale);
        weapons.addPrimaryWeaponBeam(*beam);
    }
    //left cutting beams
    for (auto& fwd_beam_pt : fwd_cutting_beams_nine) {
        vector<glm::vec3> windup{ glm::vec3(fwd_beam_pt.z,fwd_beam_pt.y,-fwd_beam_pt.x), };
        auto* beam = new BorgCuttingBeam(_this, map, windup[0], glm::vec3(-1, 0, 0), cutting_arc, windup, cutting_dmg, 4.0f, 1.5f, cutting_impact_rad, cutting_impact_time, 100.0f, 1, 6.0f, 0, cutting_end_scale, cutting_beam_scale);
        weapons.addPrimaryWeaponBeam(*beam);
    }
    //right cutting beams
    for (auto& fwd_beam_pt : fwd_cutting_beams_nine) {
        vector<glm::vec3> windup{ glm::vec3(-fwd_beam_pt.z,fwd_beam_pt.y,fwd_beam_pt.x), };
        auto* beam = new BorgCuttingBeam(_this, map, windup[0], glm::vec3(1, 0, 0), cutting_arc, windup, cutting_dmg, 4.0f, 1.5f, cutting_impact_rad, cutting_impact_time, 100.0f, 1, 6.0f, 0, cutting_end_scale, cutting_beam_scale);
        weapons.addPrimaryWeaponBeam(*beam);
    }
    //top cutting beams
    for (auto& fwd_beam_pt : fwd_cutting_beams_nine) {
        vector<glm::vec3> windup{ glm::vec3(-fwd_beam_pt.x, -fwd_beam_pt.z, -fwd_beam_pt.y), };
        auto* beam = new BorgCuttingBeam(_this, map, windup[0], glm::vec3(0, 1, 0), cutting_arc, windup, cutting_dmg, 4.0f, 1.5f, cutting_impact_rad, cutting_impact_time, 100.0f, 1, 6.0f, 0, cutting_end_scale, cutting_beam_scale);
        weapons.addPrimaryWeaponBeam(*beam);
    }
    //bottom cutting beams
    for (auto& fwd_beam_pt : fwd_cutting_beams_nine) {
        vector<glm::vec3> windup{ glm::vec3(-fwd_beam_pt.x, fwd_beam_pt.z, -fwd_beam_pt.y), };
        auto* beam = new BorgCuttingBeam(_this, map, windup[0], glm::vec3(0, -1, 0), cutting_arc, windup, cutting_dmg, 4.0f, 1.5f, cutting_impact_rad, cutting_impact_time, 100.0f, 1, 6.0f, 0, cutting_end_scale, cutting_beam_scale);
        weapons.addPrimaryWeaponBeam(*beam);
    }



    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f,0.0f, 0.0f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), weapons);
    m_AI->installThreatTable(map);
}
Cube::~Cube() {

}

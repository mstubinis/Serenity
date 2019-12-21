#include "Dderidex.h"
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

constexpr auto CLASS = "D'deridex";

using namespace std;

Dderidex::Dderidex(Scene& scene, glm::vec3 position, glm::vec3 scale)
    :Ship(CLASS, scene, position, scale) {

}

Dderidex::Dderidex(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team, client, CLASS, map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = NEW ShipSystemPitchThrusters(_this, 2.7f);
        else if (i == 2)  system = NEW ShipSystemYawThrusters(_this, 2.7f);
        else if (i == 3)  system = NEW ShipSystemRollThrusters(_this, 1.8f);
        else if (i == 4)  system = NEW ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = NEW ShipSystemShields(_this, map, 180500.0f, 180500.0f, 180500.0f, 180500.0f, 230500.0f, 230500.0f, glm::vec3(0.0f), glm::vec3(1.00f, 1.00f, 1.00f));
        else if (i == 6)  system = NEW ShipSystemMainThrusters(_this, 2.2f);
        else if (i == 7)  system = NEW ShipSystemWarpDrive(_this);
        else if (i == 8)  system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)  system = NEW ShipSystemWeapons(_this);
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 205500.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* left_claw_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(-4.81726f, 0.215001f, -2.68255), glm::vec3(0,0,-1), 17.0f);
    auto* left_claw_cannon_2 = new PlasmaCannon(_this, map, glm::vec3(-4.81726f, -0.421682f, -2.68255), glm::vec3(0, 0, -1), 17.0f);
    auto* right_claw_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(4.81726f, 0.215001f, -2.68255), glm::vec3(0, 0, -1), 17.0f);
    auto* right_claw_cannon_2 = new PlasmaCannon(_this, map, glm::vec3(4.81726f, -0.421682f, -2.68255), glm::vec3(0, 0, -1), 17.0f);
    auto* fwd_mouth_cannon = new PlasmaCannon(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 17.0f);

    vector<glm::vec3> fwd_mouth_beam_pts{
        glm::vec3(0.0f, -0.2876f, -5.98804f),
    };
    auto* fwd_mouth_beam = new PlasmaBeam(_this, map, fwd_mouth_beam_pts[0], glm::vec3(0, 0, -1), 17.0f, fwd_mouth_beam_pts, 750.0f, 4.0f, 1.05f, 2.5f, 1.8f, 100.0f, 1, 6.0f, 0, 2.0f, 2.0f);
    auto* fwd_mouth_torp_1 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_2 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_3 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_4 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_5 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_6 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_7 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);
    auto* fwd_mouth_torp_8 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 15.0f, 1);

    weapons.addPrimaryWeaponCannon(*left_claw_cannon_1, true);
    weapons.addPrimaryWeaponCannon(*left_claw_cannon_2, true);
    weapons.addPrimaryWeaponCannon(*right_claw_cannon_1, true);
    weapons.addPrimaryWeaponCannon(*right_claw_cannon_2, true);
    weapons.addPrimaryWeaponCannon(*fwd_mouth_cannon, true);
    weapons.addPrimaryWeaponBeam(*fwd_mouth_beam, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_1, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_2, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_3, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_4, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_5, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_6, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_7, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_8, true);

    auto* aft_torp_1 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* aft_torp_2 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* aft_torp_3 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* aft_torp_4 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* aft_torp_5 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* aft_torp_6 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 15.0f, 1);
    auto* aft_left_cannon = new PlasmaCannon(_this, map, glm::vec3(-0.16491f, -0.072251f, 6.94101f), glm::vec3(-0.095848f, 0, 0.724184f), 17.0f);
    auto* aft_right_cannon = new PlasmaCannon(_this, map, glm::vec3(0.16491f, -0.072251f, 6.94101f), glm::vec3(0.095848f, 0, 0.724184f), 17.0f);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_1);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_2);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_3);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_4);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_5);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_6);
    weapons.addPrimaryWeaponCannon(*aft_left_cannon);
    weapons.addPrimaryWeaponCannon(*aft_right_cannon);

    auto* starboard_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(5.07758f, -0.091561f, -1.67229f), glm::vec3(1, 0, 0), 17.0f);
    auto* port_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(-5.07758f, -0.091561f, -1.67229f), glm::vec3(-1, 0, 0), 17.0f);
    weapons.addPrimaryWeaponCannon(*starboard_cannon_1);
    weapons.addPrimaryWeaponCannon(*port_cannon_1);

    vector<glm::vec3> starboard_beam_engine_pts{
        glm::vec3(5.16708f, -0.091561f, -0.764911f),
    };
    auto* starboard_beam_engine = new PlasmaBeam(_this, map, starboard_beam_engine_pts[0], glm::vec3(1, 0, 0), 17.0f, starboard_beam_engine_pts);
    vector<glm::vec3> port_beam_engine_pts{
        glm::vec3(-5.16708f, -0.091561f, -0.764911f),
    };
    auto* port_beam_engine = new PlasmaBeam(_this, map, port_beam_engine_pts[0], glm::vec3(-1, 0, 0), 17.0f, port_beam_engine_pts);
    weapons.addPrimaryWeaponBeam(*starboard_beam_engine);
    weapons.addPrimaryWeaponBeam(*port_beam_engine);




    vector<glm::vec3> starboard_beam_tail_pts{
        glm::vec3(0.346104f, -0.073072f, 6.69949f),
    };
    auto* starboard_beam_tail = new PlasmaBeam(_this, map, starboard_beam_tail_pts[0], glm::vec3(1, 0, 0), 17.0f, starboard_beam_tail_pts);
    vector<glm::vec3> port_beam_tail_pts{
        glm::vec3(-0.346104f, -0.073072f, 6.69949f),
    };
    auto* port_beam_tail = new PlasmaBeam(_this, map, port_beam_tail_pts[0], glm::vec3(-1, 0, 0), 17.0f, port_beam_tail_pts);
    weapons.addPrimaryWeaponBeam(*starboard_beam_tail);
    weapons.addPrimaryWeaponBeam(*port_beam_tail);
    /*
    5x forward plasma cannons - done
    8x forward plasma torpedos - done
    1x forward plasma beam - done

    5x dorsal plasma beams
    2x dorsal plasma cannons
    3x ventral plasma beams
    2x ventral plasma cannons

    2x port plasma beam - done
    1x port plasma cannon - done
    2x starboard plasma beam - done
    1x starboard plasma cannon - done

    6x aft plasma torpedos - done
    2x aft plasma cannons - done

    */


    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 1.67509f, -0.389649f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Dderidex::~Dderidex() {

}
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

    //fwd cannons
    {
        auto* left_claw_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(-4.81726f, 0.215001f, -2.68255), glm::vec3(0, 0, -1), 22.0f);
        auto* left_claw_cannon_2 = new PlasmaCannon(_this, map, glm::vec3(-4.81726f, -0.421682f, -2.68255), glm::vec3(0, 0, -1), 22.0f);
        auto* right_claw_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(4.81726f, 0.215001f, -2.68255), glm::vec3(0, 0, -1), 22.0f);
        auto* right_claw_cannon_2 = new PlasmaCannon(_this, map, glm::vec3(4.81726f, -0.421682f, -2.68255), glm::vec3(0, 0, -1), 22.0f);
        auto* fwd_mouth_cannon = new PlasmaCannon(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 22.0f);
        weapons.addPrimaryWeaponCannon(*left_claw_cannon_1, true);
        weapons.addPrimaryWeaponCannon(*left_claw_cannon_2, true);
        weapons.addPrimaryWeaponCannon(*right_claw_cannon_1, true);
        weapons.addPrimaryWeaponCannon(*right_claw_cannon_2, true);
        weapons.addPrimaryWeaponCannon(*fwd_mouth_cannon, true);
    }

    //fwd torpedos
    {
        auto* fwd_mouth_torp_1 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_2 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_3 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_4 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_5 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_6 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_7 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        auto* fwd_mouth_torp_8 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.285257f, -5.89704f), glm::vec3(0, 0, -1), 19.0f, 1);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_1, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_2, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_3, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_4, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_5, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_6, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_7, true);
        weapons.addSecondaryWeaponTorpedo(*fwd_mouth_torp_8, true);
    }

    //fwd beam
    {
        vector<glm::vec3> fwd_mouth_beam_pts{ glm::vec3(0.0f, -0.2876f, -5.98804f), };
        auto* fwd_mouth_beam = new PlasmaBeam(_this, map, fwd_mouth_beam_pts[0], glm::vec3(0, 0, -1), 25.0f, fwd_mouth_beam_pts, 750.0f, 4.0f, 1.05f, 2.5f, 1.8f, 100.0f, 1, 6.0f, 0, 2.0f, 2.0f);
        weapons.addPrimaryWeaponBeam(*fwd_mouth_beam, true);
    }

    //aft torpedos
    {
        auto* aft_torp_1 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 19.0f, 1);
        auto* aft_torp_2 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 19.0f, 1);
        auto* aft_torp_3 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 19.0f, 1);
        auto* aft_torp_4 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 19.0f, 1);
        auto* aft_torp_5 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 19.0f, 1);
        auto* aft_torp_6 = new PlasmaTorpedo(_this, map, glm::vec3(0.0f, -0.072251f, 6.91773f), glm::vec3(0, 0, 1), 19.0f, 1);
        weapons.addSecondaryWeaponTorpedo(*aft_torp_1);
        weapons.addSecondaryWeaponTorpedo(*aft_torp_2);
        weapons.addSecondaryWeaponTorpedo(*aft_torp_3);
        weapons.addSecondaryWeaponTorpedo(*aft_torp_4);
        weapons.addSecondaryWeaponTorpedo(*aft_torp_5);
        weapons.addSecondaryWeaponTorpedo(*aft_torp_6);
    }

    auto* aft_left_cannon = new PlasmaCannon(_this, map, glm::vec3(-0.16491f, -0.072251f, 6.94101f), glm::vec3(-0.095848f, 0, 0.724184f), 35.0f);
    auto* aft_right_cannon = new PlasmaCannon(_this, map, glm::vec3(0.16491f, -0.072251f, 6.94101f), glm::vec3(0.095848f, 0, 0.724184f), 35.0f);
    weapons.addPrimaryWeaponCannon(*aft_left_cannon);
    weapons.addPrimaryWeaponCannon(*aft_right_cannon);

    auto* starboard_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(5.07758f, -0.091561f, -1.67229f), glm::vec3(1, 0, 0), 45.0f);
    auto* port_cannon_1 = new PlasmaCannon(_this, map, glm::vec3(-5.07758f, -0.091561f, -1.67229f), glm::vec3(-1, 0, 0), 45.0f);
    weapons.addPrimaryWeaponCannon(*starboard_cannon_1);
    weapons.addPrimaryWeaponCannon(*port_cannon_1);

    vector<glm::vec3> starboard_beam_engine_pts{ glm::vec3(5.16708f, -0.091561f, -0.764911f), };
    auto* starboard_beam_engine = new PlasmaBeam(_this, map, starboard_beam_engine_pts[0], glm::vec3(1, 0, 0), 45.0f, starboard_beam_engine_pts);
    weapons.addPrimaryWeaponBeam(*starboard_beam_engine);

    vector<glm::vec3> port_beam_engine_pts{ glm::vec3(-5.16708f, -0.091561f, -0.764911f), };
    auto* port_beam_engine = new PlasmaBeam(_this, map, port_beam_engine_pts[0], glm::vec3(-1, 0, 0), 45.0f, port_beam_engine_pts);
    weapons.addPrimaryWeaponBeam(*port_beam_engine);


    vector<glm::vec3> starboard_beam_tail_pts{ glm::vec3(0.346104f, -0.073072f, 6.69949f), };
    auto* starboard_beam_tail = new PlasmaBeam(_this, map, starboard_beam_tail_pts[0], glm::vec3(1, 0, 0), 27.0f, starboard_beam_tail_pts);
    weapons.addPrimaryWeaponBeam(*starboard_beam_tail);

    vector<glm::vec3> port_beam_tail_pts{ glm::vec3(-0.346104f, -0.073072f, 6.69949f), };
    auto* port_beam_tail = new PlasmaBeam(_this, map, port_beam_tail_pts[0], glm::vec3(-1, 0, 0), 27.0f, port_beam_tail_pts);
    weapons.addPrimaryWeaponBeam(*port_beam_tail);


    //dorsal neck beam
    vector<glm::vec3> dosal_neck_beam_pts{ glm::vec3(0.0f, 1.75238f, -3.31606f), };
    auto* dosal_neck_beam = new PlasmaBeam(_this, map, dosal_neck_beam_pts[0], glm::vec3(0.0f, 1.92398f, -0.293939f), 40.0f, dosal_neck_beam_pts);
    weapons.addPrimaryWeaponBeam(*dosal_neck_beam);


    //left central dorsal beam
    vector<glm::vec3> left_central_dorsal_beam_pts{ glm::vec3(-0.629328f, 1.80215f, 0.070756f), };
    auto* left_central_dorsal_beam = new PlasmaBeam(_this, map, left_central_dorsal_beam_pts[0], glm::vec3(-0.160514f, 0.306768f, 0.0f), 40.0f, left_central_dorsal_beam_pts);
    weapons.addPrimaryWeaponBeam(*left_central_dorsal_beam);

    //right central dorsal beam
    vector<glm::vec3> right_central_dorsal_beam_pts{ glm::vec3(0.629328f, 1.80215f, 0.070756f), };
    auto* right_central_dorsal_beam = new PlasmaBeam(_this, map, right_central_dorsal_beam_pts[0], glm::vec3(0.160514f, 0.306768f, 0.0f), 40.0f, right_central_dorsal_beam_pts);
    weapons.addPrimaryWeaponBeam(*right_central_dorsal_beam);


    //left aft dorsal beam
    vector<glm::vec3> left_aft_dorsal_beam_pts{ glm::vec3(-1.5611f, 1.03447f, 3.81863f), };
    auto* left_aft_dorsal_beam = new PlasmaBeam(_this, map, left_aft_dorsal_beam_pts[0], glm::vec3(-0.243891f, 0.548772f, 0.261456f), 40.0f, left_aft_dorsal_beam_pts);
    weapons.addPrimaryWeaponBeam(*left_aft_dorsal_beam);

    //right aft dorsal beam
    vector<glm::vec3> right_aft_dorsal_beam_pts{ glm::vec3(1.5611f, 1.03447f, 3.81863f), };
    auto* right_aft_dorsal_beam = new PlasmaBeam(_this, map, right_aft_dorsal_beam_pts[0], glm::vec3(0.243891f, 0.548772f, 0.261456f), 40.0f, right_aft_dorsal_beam_pts);
    weapons.addPrimaryWeaponBeam(*right_aft_dorsal_beam);


    auto* left_dorsal_cannon = new PlasmaCannon(_this, map, glm::vec3(-2.16005f, 1.35721f, 1.31413f), glm::vec3(-0.107363f, 1.29862f, 0.107363f), 37.0f);
    auto* right_dorsal_cannon = new PlasmaCannon(_this, map, glm::vec3(2.16005f, 1.35721f, 1.31413f), glm::vec3(0.107363f, 1.29862f, 0.107363f), 37.0f);
    weapons.addPrimaryWeaponCannon(*left_dorsal_cannon);
    weapons.addPrimaryWeaponCannon(*right_dorsal_cannon);



    //left aft ventral beam
    vector<glm::vec3> left_aft_ventral_beam_pts{ glm::vec3(-1.5526f, -1.18005f, 3.80477f), };
    auto* left_aft_ventral_beam = new PlasmaBeam(_this, map, left_aft_ventral_beam_pts[0], glm::vec3(-0.686168f, -1.51295f, 0.629223f), 40.0f, left_aft_ventral_beam_pts);
    weapons.addPrimaryWeaponBeam(*left_aft_ventral_beam);

    //right aft ventral beam
    vector<glm::vec3> right_aft_ventral_beam_pts{ glm::vec3(1.5526f, -1.18005f, 3.80477f), };
    auto* right_aft_ventral_beam = new PlasmaBeam(_this, map, right_aft_ventral_beam_pts[0], glm::vec3(0.686168f, -1.51295f, 0.629223f), 40.0f, right_aft_ventral_beam_pts);
    weapons.addPrimaryWeaponBeam(*right_aft_ventral_beam);


    //central ventral beam
    vector<glm::vec3> central_ventral_beam_pts{ glm::vec3(0.0f, -1.76653f, -0.195749f), };
    auto* central_ventral_beam = new PlasmaBeam(_this, map, central_ventral_beam_pts[0], glm::vec3(0.0f, -0.831758f, 0.080163f), 40.0f, central_ventral_beam_pts);
    weapons.addPrimaryWeaponBeam(*central_ventral_beam);


    //left ventral cannon
    auto* left_ventral_cannon = new PlasmaCannon(_this, map, glm::vec3(-1.69542f, -1.43601f, 1.41225f), glm::vec3(-0.052211f, -0.693372f, 0.063347f), 37.0f);
    weapons.addPrimaryWeaponCannon(*left_ventral_cannon);

    //right ventral cannon
    auto* right_ventral_cannon = new PlasmaCannon(_this, map, glm::vec3(1.69542f, -1.43601f, 1.41225f), glm::vec3(0.052211f, -0.693372f, 0.063347f), 37.0f);
    weapons.addPrimaryWeaponCannon(*right_ventral_cannon);

    /*
    5x forward plasma cannons - done
    8x forward plasma torpedos - done
    1x forward plasma beam - done

    5x dorsal plasma beams - done
    2x dorsal plasma cannons - done
    3x ventral plasma beams - done
    2x ventral plasma cannons - done

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

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), weapons);
    m_AI->installThreatTable(map);
}
Dderidex::~Dderidex() {

}
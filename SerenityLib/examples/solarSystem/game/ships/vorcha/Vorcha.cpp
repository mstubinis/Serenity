#include "Vorcha.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/DisruptorCannon.h"
#include "../../weapons/KlingonPhotonTorpedo.h"
#include "../../weapons/DisruptorBeam.h"

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

using namespace std;

Vorcha::Vorcha(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, "Vor'cha", map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = new ShipSystemPitchThrusters(_this, 1.25f);
        else if (i == 2)   system = new ShipSystemYawThrusters(_this, 1.25f);
        else if (i == 3)   system = new ShipSystemRollThrusters(_this, 1.17f);
        else if (i == 4)   system = new ShipSystemCloakingDevice(_this);
        else if (i == 5)   system = new ShipSystemShields(_this, map, 21500.0f, 21500.0f, 21500.0f, 21500.0f, 30000.0f, 30000.0f, glm::vec3(0.0f, -0.107706f, 0.279445f), glm::vec3(1.0f,1.3f,1.0f));
        else if (i == 6)   system = new ShipSystemMainThrusters(_this, 1.15f);
        else if (i == 7)   system = new ShipSystemWarpDrive(_this);
        else if (i == 8)   system = new ShipSystemSensors(_this, map);
        else if (i == 9)   system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 51500.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    vector<glm::vec3> forward_heavy_beam_windup{
        glm::vec3(0, 0.080577f, -2.27926f),
    };
    auto* forward_heavy_beam = new DisruptorBeam(_this, map, glm::vec3(0, 0.080577f, -2.27926f), glm::vec3(0, 0, -1), 25.0f, forward_heavy_beam_windup, 1100.0f, 4.0f, 1.05f, 2.5f, 1.8f, 100.0f, 1, 5.0f, 0, 2.2f, 2.2f, 10.0f);

    auto* forward_front_right_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(0.159582f, 0.080577f, -2.47036f), glm::vec3(0, 0, -1), 25.0f, 4, 200.0f);
    auto* forward_front_left_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(-0.159582f, 0.080577f, -2.47036f), glm::vec3(0, 0, -1), 25.0f, 4, 200.0f);

    weapons.addPrimaryWeaponBeam(*forward_heavy_beam, true);
    weapons.addPrimaryWeaponCannon(*forward_front_right_dis_cannon, true);
    weapons.addPrimaryWeaponCannon(*forward_front_left_dis_cannon, true);

    auto* forward_torp_1 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, -0.093022f, -0.729978f), glm::vec3(0, 0, -1), 20.0f, 1);
    auto* forward_torp_2 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, -0.093022f, -0.729978f), glm::vec3(0, 0, -1), 20.0f, 1);
    auto* forward_torp_3 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, -0.093022f, -0.729978f), glm::vec3(0, 0, -1), 20.0f, 1);
    auto* forward_torp_4 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, -0.093022f, -0.729978f), glm::vec3(0, 0, -1), 20.0f, 1);
    auto* forward_torp_5 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, -0.093022f, -0.729978f), glm::vec3(0, 0, -1), 20.0f, 1);
    weapons.addSecondaryWeaponTorpedo(*forward_torp_1, true);
    weapons.addSecondaryWeaponTorpedo(*forward_torp_2, true);
    weapons.addSecondaryWeaponTorpedo(*forward_torp_3, true);
    weapons.addSecondaryWeaponTorpedo(*forward_torp_4, true);
    weapons.addSecondaryWeaponTorpedo(*forward_torp_5, true);



    auto* aft_left_torp_1 = new KlingonPhotonTorpedo(_this, map, glm::vec3(-0.072712f, 0.324057f, 1.16697f), glm::vec3(0, 0, 1), 20.0f, 1);
    auto* aft_right_torp_1 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.072712f, 0.324057f, 1.16697f), glm::vec3(0, 0, 1), 20.0f, 1);
    auto* aft_left_torp_2 = new KlingonPhotonTorpedo(_this, map, glm::vec3(-0.072712f, 0.324057f, 1.16697f), glm::vec3(0, 0, 1), 20.0f, 1);
    auto* aft_right_torp_2 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.072712f, 0.324057f, 1.16697f), glm::vec3(0, 0, 1), 20.0f, 1);
    weapons.addSecondaryWeaponTorpedo(*aft_left_torp_1);
    weapons.addSecondaryWeaponTorpedo(*aft_right_torp_1);
    weapons.addSecondaryWeaponTorpedo(*aft_left_torp_2);
    weapons.addSecondaryWeaponTorpedo(*aft_right_torp_2);

    auto* front_port_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(-0.537247f, 0.074084f, 0.139261f), glm::vec3(-0.697538f, 0, -0.755028f), 32.0f, 3, 200.0f);
    auto* front_star_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(0.537247f, 0.074084f, 0.139261f), glm::vec3(0.697538f, 0, -0.755028f), 32.0f, 3, 200.0f);
    weapons.addPrimaryWeaponCannon(*front_port_dis_cannon);
    weapons.addPrimaryWeaponCannon(*front_star_dis_cannon);


    auto* port_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(-1.42054f, -0.014244f, 0.857809f), glm::vec3(-1, 0, 0), 32.0f, 3, 200.0f);
    auto* star_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(1.42054f, -0.014244f, 0.857809f), glm::vec3(1, 0, 0), 32.0f, 3, 200.0f);
    weapons.addPrimaryWeaponCannon(*port_dis_cannon);
    weapons.addPrimaryWeaponCannon(*star_dis_cannon);

    vector<glm::vec3> port_dis_beam_windup{
        glm::vec3(-1.42054f, -0.014244f, 0.857809f),
    };
    vector<glm::vec3> star_dis_beam_windup{
        glm::vec3(1.42054f, -0.014244f, 0.857809f),
    };
    auto* port_dis_beam = new DisruptorBeam(_this, map, glm::vec3(-1.42054f, -0.014244f, 0.857809f), glm::vec3(-1, 0, 0), 38.0f, port_dis_beam_windup, 1100.0f, 4.0f, 1.05f, 2.5f, 1.8f, 100.0f, 1, 5.0f, 0, 1.0f, 1.0f, 10.0f);
    auto* star_dis_beam = new DisruptorBeam(_this, map, glm::vec3(1.42054f, -0.014244f, 0.857809f), glm::vec3(1, 0, 0), 38.0f, star_dis_beam_windup, 1100.0f, 4.0f, 1.05f, 2.5f, 1.8f, 100.0f, 1, 5.0f, 0, 1.0f, 1.0f, 10.0f);
    weapons.addPrimaryWeaponBeam(*port_dis_beam);
    weapons.addPrimaryWeaponBeam(*star_dis_beam);


    auto* aft_left_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(-0.693038f, 0.083061f, 1.38455f), glm::vec3(-0.058693f, 0, 0.736922f), 32.0f, 3, 200.0f);
    auto* aft_right_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(0.693038f, 0.083061f, 1.38455f), glm::vec3(0.058693f, 0, 0.736922f), 32.0f, 3, 200.0f);
    weapons.addPrimaryWeaponCannon(*aft_left_dis_cannon);
    weapons.addPrimaryWeaponCannon(*aft_right_dis_cannon);


    auto* dorsal_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(0.0f, 0.535206f, 1.0818f), glm::vec3(0.0f, 0.958169f, -0.169276f), 45.0f, 3, 200.0f);
    auto* ventral_dis_cannon = new DisruptorCannon(_this, map, glm::vec3(0.0f, -0.080504f, -0.086177f), glm::vec3(0.0f, -1.0f, 0.0f), 45.0f, 3, 200.0f);
    weapons.addPrimaryWeaponCannon(*dorsal_dis_cannon);
    weapons.addPrimaryWeaponCannon(*ventral_dis_cannon);

    if (Ships::Database["Vor'cha"].HullImpactPoints.size() == 0) {
        Ships::Database["Vor'cha"].HullImpactPoints = {
            glm::vec3(0,0.07566f, -2.00476f),
            glm::vec3(0,0.07566f, -1.62338f),
            glm::vec3(0,0.07566f, -1.38325f),
            glm::vec3(0,0.07566f, -1.15317f),
            glm::vec3(0,0.07566f, -0.870319f),
            glm::vec3(0,0.07566f, -0.614861f),
            glm::vec3(0,0.07566f, -0.326837f),
            glm::vec3(0,0.07566f, -0.015476f),
            glm::vec3(0,0.07566f, 0.18899f),
            glm::vec3(0,0.07566f, 0.446896),
            glm::vec3(0,0.07566f, 0.80229f),
            glm::vec3(0,0.07566f, 1.1861f),
            glm::vec3(0,0.456266f, 1.0699f),
            glm::vec3(0,0.456266f, 0.637626f),
            glm::vec3(0,0.456266f, 0.842013f),
            glm::vec3(0.154628f,0.456266f, 1.07265f),
            glm::vec3(-0.154628f,0.456266f, 1.07265f),
            glm::vec3(-0.083866f,0.456266f, 0.844639f),
            glm::vec3(0.083866f,0.456266f, 0.844639f),
            glm::vec3(0.0f,0.207717f, -0.111021f),
            glm::vec3(0.0f,0.207717f, 0.136501f),
            glm::vec3(0.0f,0.207717f, 0.327603f),
            glm::vec3(0.0f,0.207717f, 0.456839f),
            glm::vec3(-0.093913f,0.207717f, 0.231885f),
            glm::vec3(0.093913f,0.207717f, 0.231885f),
            glm::vec3(-0.049929f,0.207717f, 0.057467f),
            glm::vec3(0.049929f,0.207717f, 0.057467f),
            glm::vec3(-0.139474f,0.207717f, 0.542319f),
            glm::vec3(0.139474f,0.207717f, 0.542319f),
            glm::vec3(-0.266727f,0.268545f, 0.59935f),
            glm::vec3(0.266727f,0.268545f, 0.59935f),
            glm::vec3(-0.304644f,0.268545f, 0.824436f),
            glm::vec3(0.304644f,0.268545f, 0.824436f),
            glm::vec3(0.451762f,0.188156f, 0.630301f),
            glm::vec3(-0.451762f,0.188156f, 0.630301f),
            glm::vec3(-0.416878f,0.188156f, 1.05194f),
            glm::vec3(0.416878f,0.188156f, 1.05194f),
            glm::vec3(-0.563996f,0.188156f, 0.854769f),
            glm::vec3(0.563996f,0.188156f, 0.854769f),
            glm::vec3(-0.405489f,0.093051f, 0.302258f),
            glm::vec3(0.405489f,0.093051f, 0.302258f),
            glm::vec3(-0.779742f,0.093051f,0.654496f),
            glm::vec3(0.779742f,0.093051f, 0.654496f),
            glm::vec3(-0.930701f,0.093051f,1.10737f),
            glm::vec3(0.930701f,0.093051f, 1.10737f),
            glm::vec3(-1.16028f,0.030147f,0.893511f),
            glm::vec3(1.16028f,0.030147f, 0.893511f),
            glm::vec3(-1.35415f,-0.042981f,0.77962f),
            glm::vec3(1.35415f,-0.042981f, 0.77962f),
            glm::vec3(-1.39207f,-0.108703f,1.02956f),
            glm::vec3(1.39207f,-0.108703f, 1.02956f),
            glm::vec3(-1.54303f,-0.291114f,0.255894f),
            glm::vec3(1.54303f,-0.291114f, 0.255894f),
            glm::vec3(-1.54303f,-0.291114f,0.654426f),
            glm::vec3(1.54303f,-0.291114f, 0.654426f),
            glm::vec3(-1.54303f,-0.291114f,1.03937f),
            glm::vec3(1.54303f,-0.291114f, 1.03937f),
            glm::vec3(-1.54303f,-0.291114f,1.46508f),
            glm::vec3(1.54303f,-0.291114f, 1.46508f),
            glm::vec3(-1.54303f,-0.291114f,1.91796f),
            glm::vec3(1.54303f,-0.291114f, 1.91796f),
            glm::vec3(-0.16928f,0.111264f,-0.533059f),
            glm::vec3(0.16928f,0.111264f, -0.533059f),
            glm::vec3(-0.098518f,0.111264f,-0.829212f),
            glm::vec3(0.098518f,0.111264f, -0.829212f),
            glm::vec3(0.0f,0.303733f, -1.15937f),
            glm::vec3(-0.169261f,0.303733f, -1.18485f),
            glm::vec3(0.169261f,0.303733f, -1.18485f),
            glm::vec3(-0.106361f,0.080437f, -1.54652f),
            glm::vec3(0.106361f,0.080437f, -1.54652f),
            glm::vec3(-0.210146f,0.080437f, -1.7478f),
            glm::vec3(0.210146f,0.080437f, -1.7478f),
            glm::vec3(-0.169261f,0.080437f, -1.98682f),
            glm::vec3(-0.169261f,0.080437f, -1.98682f),
            glm::vec3(-0.181841f,0.080437f, -2.20382f),
            glm::vec3(0.181841f,0.080437f, -2.20382f),
            glm::vec3(0,-0.035834f, -1.87929f),
            glm::vec3(0,-0.058478f, -0.662187f),
            glm::vec3(0,-0.044892f, -0.430967f),
            glm::vec3(0,-0.044892f, -0.044697f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Vorcha::~Vorcha() {
}
#include "Akira.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/torpedos/PhotonTorpedo.h"
#include "../../weapons/beams/PhaserBeam.h"

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

constexpr auto CLASS = "Akira";

using namespace std;

Akira::Akira(Scene& scene, glm::vec3 position, glm::vec3 scale)
:Ship(CLASS, scene, position, scale) {

}

Akira::Akira(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS, map, ai_type, name, position, scale, collisionType) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem*        system = nullptr;
        if (i == 0)        system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = NEW ShipSystemPitchThrusters(_this);
        else if (i == 2)   system = NEW ShipSystemYawThrusters(_this);
        else if (i == 3)   system = NEW ShipSystemRollThrusters(_this);
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = NEW ShipSystemShields(_this, map, 25500.0f, 25500.0f, 25500.0f, 25500.0f, 32500.0f, 32500.0f);
        else if (i == 6)   system = NEW ShipSystemMainThrusters(_this);
        else if (i == 7)   system = NEW ShipSystemWarpDrive(_this);
        else if (i == 8)   system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)   system = NEW ShipSystemWeapons(_this);
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 55500.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //rack torpedos
    auto* top_torp_1 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.127431f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_2 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.041798f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_3 = NEW PhotonTorpedo(_this, map, glm::vec3(0.041798f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_4 = NEW PhotonTorpedo(_this, map, glm::vec3(0.127431f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_5 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.076881f, 0.370706f, 0.030954f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_6 = NEW PhotonTorpedo(_this, map, glm::vec3(0.0f, 0.370706f, 0.030954f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_7 = NEW PhotonTorpedo(_this, map, glm::vec3(0.076881f, 0.370706f, 0.030954f), glm::vec3(0, 0, -1), 12.0f);
    //ventral torp
    auto* ventral_torp_8 = NEW PhotonTorpedo(_this, map, glm::vec3(0.0f, -0.078411f, -2.47198f), glm::vec3(0, 0, -1), 12.0f);
    //left torps
    auto* left_torp_1 = NEW PhotonTorpedo(_this, map, glm::vec3(-1.2734f, 0.038987f, -1.66943f), glm::vec3(-1, 0, 0), 12.0f);
    auto* left_torp_2 = NEW PhotonTorpedo(_this, map, glm::vec3(-1.2734f, 0.038987f, -1.60432f), glm::vec3(-1, 0, 0), 12.0f);
    //right torps
    auto* right_torp_1 = NEW PhotonTorpedo(_this, map, glm::vec3(1.2734f, 0.038987f, -1.66943f), glm::vec3(1, 0, 0), 12.0f);
    auto* right_torp_2 = NEW PhotonTorpedo(_this, map, glm::vec3(1.2734f, 0.038987f, -1.60432f), glm::vec3(1, 0, 0), 12.0f);
    //aft rack torps
    auto* aft_torp_1 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.233761f, 0.503108f, 0.736298f), glm::vec3(-0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_2 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.177242f, 0.503108f, 0.797487f), glm::vec3(-0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_3 = NEW PhotonTorpedo(_this, map, glm::vec3(-0.118947f, 0.503108f, 0.860294f), glm::vec3(-0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_4 = NEW PhotonTorpedo(_this, map, glm::vec3(0.233761f, 0.503108f, 0.736298f), glm::vec3(0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_5 = NEW PhotonTorpedo(_this, map, glm::vec3(0.177242f, 0.503108f, 0.797487f), glm::vec3(0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_6 = NEW PhotonTorpedo(_this, map, glm::vec3(0.118947f, 0.503108f, 0.860294f), glm::vec3(0.26491f, 0, 0.318563f), 20.0f);


    weapons.addSecondaryWeaponTorpedo(*top_torp_1, true);
    weapons.addSecondaryWeaponTorpedo(*top_torp_2, true);
    weapons.addSecondaryWeaponTorpedo(*top_torp_3, true);
    weapons.addSecondaryWeaponTorpedo(*top_torp_4, true);
    weapons.addSecondaryWeaponTorpedo(*top_torp_5, true);
    weapons.addSecondaryWeaponTorpedo(*top_torp_6, true);
    weapons.addSecondaryWeaponTorpedo(*top_torp_7, true);
    weapons.addSecondaryWeaponTorpedo(*ventral_torp_8, true);

    weapons.addSecondaryWeaponTorpedo(*left_torp_1);
    weapons.addSecondaryWeaponTorpedo(*left_torp_2);
    weapons.addSecondaryWeaponTorpedo(*right_torp_1);
    weapons.addSecondaryWeaponTorpedo(*right_torp_2);

    weapons.addSecondaryWeaponTorpedo(*aft_torp_1);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_2);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_3);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_4);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_5);
    weapons.addSecondaryWeaponTorpedo(*aft_torp_6);


    //top phasers
    //top left left
    vector<glm::vec3> top_left_left{
        glm::vec3(-1.12758f, 0.08336f, -1.35959f),
        glm::vec3(-1.1673f, 0.083752f, -1.64135f),
        glm::vec3(-1.12706f, 0.083752f, -1.91631f),
        glm::vec3(-1.01641f, 0.083752f, -2.16446f),
        glm::vec3(-0.845388f, 0.083752f, -2.36566f),
    };
    //top front left
    vector<glm::vec3> top_front_left{
        glm::vec3(-0.845388f, 0.083752f, -2.36566f),
        glm::vec3(-0.65425f, 0.083752f, -2.50985f),
        glm::vec3(-0.4497f, 0.083752f, -2.62386f),
        glm::vec3(-0.231736f, 0.083752f, -2.68758f),
        glm::vec3(0.0f, 0.082952f, -2.70664f),
    };
    //top right right
    vector<glm::vec3> top_right_right{
        glm::vec3(1.12758f, 0.08336f, -1.35959f),
        glm::vec3(1.1673f, 0.083752f, -1.64135f),
        glm::vec3(1.12706f, 0.083752f, -1.91631f),
        glm::vec3(1.01641f, 0.083752f, -2.16446f),
        glm::vec3(0.845388f, 0.083752f, -2.36566f),
    };
    //top front right
    vector<glm::vec3> top_front_right{
        glm::vec3(0.845388f, 0.083752f, -2.36566f),
        glm::vec3(0.65425f, 0.083752f, -2.50985f),
        glm::vec3(0.4497f, 0.083752f, -2.62386f),
        glm::vec3(0.231736f, 0.083752f, -2.68758f),
        glm::vec3(0.0f, 0.082952f, -2.70664f),
    };
    auto* top_left_left_beam = NEW PhaserBeam(_this, map, top_left_left[2], glm::vec3(-0.492934f, 0.249497f, -0.221317f), 40.0f, top_left_left);
    auto* top_front_left_beam = NEW PhaserBeam(_this, map, top_front_left[2], glm::vec3(-0.24479f, 0.249497f, -0.342036f), 40.0f, top_front_left);
    auto* top_right_right_beam = NEW PhaserBeam(_this, map, top_right_right[2], glm::vec3(0.492934f, 0.249497f, -0.221317f), 40.0f, top_right_right);
    auto* top_front_right_beam = NEW PhaserBeam(_this, map, top_front_right[2], glm::vec3(0.24479f, 0.249497f, -0.342036f), 40.0f, top_front_right);

    weapons.addPrimaryWeaponBeam(*top_left_left_beam);
    weapons.addPrimaryWeaponBeam(*top_front_left_beam, true);
    weapons.addPrimaryWeaponBeam(*top_right_right_beam);
    weapons.addPrimaryWeaponBeam(*top_front_right_beam, true);

    //bottom phasers
    vector<glm::vec3> bottom_left{
        glm::vec3(1.09873f, -0.061473f, -1.45503f),
        glm::vec3(1.08515f, -0.061473f, -1.80433f),
        glm::vec3(0.943489f, -0.061473f, -2.14393f),
        glm::vec3(0.714502f, -0.061473f, -2.40396f),
        glm::vec3(0.419537f, -0.061473f, -2.57861f),
    };
    vector<glm::vec3> bottom_right{
        glm::vec3(-1.09873f, -0.061473f, -1.45503f),
        glm::vec3(-1.08515f, -0.061473f, -1.80433f),
        glm::vec3(-0.943489f, -0.061473f, -2.14393f),
        glm::vec3(-0.714502f, -0.061473f, -2.40396f),
        glm::vec3(-0.419537f, -0.061473f, -2.57861f),
    };
    auto* btm_left_left_beam = NEW PhaserBeam(_this, map, bottom_left[2], glm::vec3(0.301313f, -0.353465f, -0.197012f), 60.0f, bottom_left);
    auto* btm_front_left_beam = NEW PhaserBeam(_this, map, bottom_right[2], glm::vec3(-0.301313f, -0.353465f, -0.197012f), 60.0f, bottom_right);
    weapons.addPrimaryWeaponBeam(*btm_left_left_beam, true);
    weapons.addPrimaryWeaponBeam(*btm_front_left_beam, true);


    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 0.0f, -1.58693f),
            glm::vec3(-0.613995f, 0.027912f, -1.6191f),
            glm::vec3(0.613995f, 0.027912f, -1.6191f),
            glm::vec3(-0.38463f, 0.027912f, -2.04564f),
            glm::vec3(0.38463f, 0.027912f, -2.04564f),
            glm::vec3(0.0f, 0.000098f, -2.41808f),
            glm::vec3(-0.535411f, 0.201759f, -0.839661f),
            glm::vec3(0.535411f, 0.201759f, -0.839661f),
            glm::vec3(-0.810765f, 0.253006f, 0.720679f),
            glm::vec3(0.810765f, 0.253006f, 0.720679f),
            glm::vec3(0.0f, 0.455842f, -0.487045f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), weapons);
    m_AI->installThreatTable(map);
}
Akira::~Akira() {

}

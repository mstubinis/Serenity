#include "Akira.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
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

using namespace std;

Akira::Akira(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
    :Ship(client, ResourceManifest::AkiraMesh, ResourceManifest::AkiraMaterial, "Akira", map, player, name, position, scale, collisionType, glm::vec3(0.0f, 0.0f, -1.58693f)) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem*        system = nullptr;
        if (i == 0)        system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)   system = new ShipSystemYawThrusters(_this);
        else if (i == 3)   system = new ShipSystemRollThrusters(_this);
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = new ShipSystemShields(_this, map, 25500.0f, 25500.0f, 25500.0f, 25500.0f, 32500.0f, 32500.0f);
        else if (i == 6)   system = new ShipSystemMainThrusters(_this);
        else if (i == 7)   system = new ShipSystemWarpDrive(_this);
        else if (i == 8)   system = new ShipSystemSensors(_this, map);
        else if (i == 9)   system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 38500.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //rack torpedos
    auto* top_torp_1 = new PhotonTorpedo(_this, map, glm::vec3(-0.127431f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_2 = new PhotonTorpedo(_this, map, glm::vec3(-0.041798f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_3 = new PhotonTorpedo(_this, map, glm::vec3(0.041798f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_4 = new PhotonTorpedo(_this, map, glm::vec3(0.127431f, 0.497265f, 0.081451f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_5 = new PhotonTorpedo(_this, map, glm::vec3(-0.076881f, 0.370706f, 0.030954f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_6 = new PhotonTorpedo(_this, map, glm::vec3(0.0f, 0.370706f, 0.030954f), glm::vec3(0, 0, -1), 12.0f);
    auto* top_torp_7 = new PhotonTorpedo(_this, map, glm::vec3(0.076881f, 0.370706f, 0.030954f), glm::vec3(0, 0, -1), 12.0f);
    //ventral torp
    auto* ventral_torp_8 = new PhotonTorpedo(_this, map, glm::vec3(0.0f, -0.078411f, -2.47198f), glm::vec3(0, 0, -1), 12.0f);
    //left torps
    auto* left_torp_1 = new PhotonTorpedo(_this, map, glm::vec3(-1.2734f, 0.038987f, -1.66943f), glm::vec3(-1, 0, 0), 12.0f);
    auto* left_torp_2 = new PhotonTorpedo(_this, map, glm::vec3(-1.2734f, 0.038987f, -1.60432f), glm::vec3(-1, 0, 0), 12.0f);
    //right torps
    auto* right_torp_1 = new PhotonTorpedo(_this, map, glm::vec3(1.2734f, 0.038987f, -1.66943f), glm::vec3(1, 0, 0), 12.0f);
    auto* right_torp_2 = new PhotonTorpedo(_this, map, glm::vec3(1.2734f, 0.038987f, -1.60432f), glm::vec3(1, 0, 0), 12.0f);
    //aft rack torps
    auto* aft_torp_1 = new PhotonTorpedo(_this, map, glm::vec3(-0.233761f, 0.503108f, 0.736298f), glm::vec3(-0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_2 = new PhotonTorpedo(_this, map, glm::vec3(-0.177242f, 0.503108f, 0.797487f), glm::vec3(-0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_3 = new PhotonTorpedo(_this, map, glm::vec3(-0.118947f, 0.503108f, 0.860294f), glm::vec3(-0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_4 = new PhotonTorpedo(_this, map, glm::vec3(0.233761f, 0.503108f, 0.736298f), glm::vec3(0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_5 = new PhotonTorpedo(_this, map, glm::vec3(0.177242f, 0.503108f, 0.797487f), glm::vec3(0.26491f, 0, 0.318563f), 20.0f);
    auto* aft_torp_6 = new PhotonTorpedo(_this, map, glm::vec3(0.118947f, 0.503108f, 0.860294f), glm::vec3(0.26491f, 0, 0.318563f), 20.0f);


    weapons.addSecondaryWeaponTorpedo(*top_torp_1);
    weapons.addSecondaryWeaponTorpedo(*top_torp_2);
    weapons.addSecondaryWeaponTorpedo(*top_torp_3);
    weapons.addSecondaryWeaponTorpedo(*top_torp_4);
    weapons.addSecondaryWeaponTorpedo(*top_torp_5);
    weapons.addSecondaryWeaponTorpedo(*top_torp_6);
    weapons.addSecondaryWeaponTorpedo(*top_torp_7);
    weapons.addSecondaryWeaponTorpedo(*ventral_torp_8);

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
    auto* top_left_left_beam = new PhaserBeam(_this, map, top_left_left[2], glm::vec3(-0.492934f, 0.249497f, -0.221317f), 40.0f, top_left_left);
    auto* top_front_left_beam = new PhaserBeam(_this, map, top_front_left[2], glm::vec3(-0.24479f, 0.249497f, -0.342036f), 40.0f, top_front_left);
    auto* top_right_right_beam = new PhaserBeam(_this, map, top_right_right[2], glm::vec3(0.492934f, 0.249497f, -0.221317f), 40.0f, top_right_right);
    auto* top_front_right_beam = new PhaserBeam(_this, map, top_front_right[2], glm::vec3(0.24479f, 0.249497f, -0.342036f), 40.0f, top_front_right);

    weapons.addPrimaryWeaponBeam(*top_left_left_beam);
    weapons.addPrimaryWeaponBeam(*top_front_left_beam);
    weapons.addPrimaryWeaponBeam(*top_right_right_beam);
    weapons.addPrimaryWeaponBeam(*top_front_right_beam);

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
    auto* btm_left_left_beam = new PhaserBeam(_this, map, bottom_left[2], glm::vec3(0.301313f, -0.353465f, -0.197012f), 60.0f, bottom_left);
    auto* btm_front_left_beam = new PhaserBeam(_this, map, bottom_right[2], glm::vec3(-0.301313f, -0.353465f, -0.197012f), 60.0f, bottom_right);
    weapons.addPrimaryWeaponBeam(*btm_left_left_beam);
    weapons.addPrimaryWeaponBeam(*btm_front_left_beam);

    //TODO: fill this info for the rest of the ships
    //random hull target points
    vector<glm::vec3> hull_target_points = {
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
    addHullTargetPoints(hull_target_points);
}
Akira::~Akira() {

}

#include "Miranda.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PhotonTorpedoOld.h"
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

Miranda::Miranda(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client, "Miranda", map, player, name, position, scale, collisionType, glm::vec3(0.0f, 0.128851f, -0.125263f), glm::vec3(0.0f, 0.1f, 0.1f)) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this);
        else if (i == 4)  system = nullptr; //no cloaking device
        else if (i == 5)  system = new ShipSystemShields(_this, map, 8500.0f, 8500.0f, 8500.0f, 8500.0f, 11500.0f, 11500.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 8000.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTorp = new PhotonTorpedoOld(_this, map, glm::vec3(-0.03427f, 0.310924f, 0.07898f), glm::vec3(0, 0, -1), 15.0f);
    auto* rightTorp = new PhotonTorpedoOld(_this, map, glm::vec3(0.03427f, 0.310924f, 0.07898f), glm::vec3(0, 0, -1), 15.0f);
    auto* aftTorpLeft = new PhotonTorpedoOld(_this, map, glm::vec3(-0.052569f, 0.306842f, 0.405438f), glm::vec3(0, 0, 1), 15.0f);
    auto* aftTorpRight = new PhotonTorpedoOld(_this, map, glm::vec3(0.052569f, 0.306842f, 0.405438f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addSecondaryWeaponTorpedo(*leftTorp);
    weapons.addSecondaryWeaponTorpedo(*rightTorp);
    weapons.addSecondaryWeaponTorpedo(*aftTorpLeft);
    weapons.addSecondaryWeaponTorpedo(*aftTorpRight);

    //phasers
    vector<glm::vec3> front_pylon_left_pts{ glm::vec3(-0.531989f, 0.264319f, -0.010741f) };
    vector<glm::vec3> front_pylon_right_pts{ glm::vec3(0.531989f, 0.264319f, -0.010741f) };

    vector<glm::vec3> aft_pylon_left_pts{ glm::vec3(-0.531989f, 0.264319f, 0.274019f) };
    vector<glm::vec3> aft_pylon_right_pts{ glm::vec3(0.531989f, 0.264319f, 0.274019f) };

    auto* front_pylon_left = new PhaserBeam(_this, map, front_pylon_left_pts[0], glm::vec3(0, 0, -1), 10.0f, front_pylon_left_pts);
    auto* front_pylon_right = new PhaserBeam(_this, map, front_pylon_right_pts[0], glm::vec3(0, 0, -1), 10.0f, front_pylon_right_pts);

    auto* aft_pylon_left = new PhaserBeam(_this, map, aft_pylon_left_pts[0], glm::vec3(0, 0, 1), 10.0f, aft_pylon_left_pts);
    auto* aft_pylon_right = new PhaserBeam(_this, map, aft_pylon_right_pts[0], glm::vec3(0, 0, 1), 10.0f, aft_pylon_right_pts);

    weapons.addPrimaryWeaponBeam(*front_pylon_left);
    weapons.addPrimaryWeaponBeam(*front_pylon_right);
    weapons.addPrimaryWeaponBeam(*aft_pylon_left);
    weapons.addPrimaryWeaponBeam(*aft_pylon_right);

    //aft impulse engine phasers, not sure about this but it seems to be canon, but also overpowered
    vector<glm::vec3> impulse_beam_left_pts{ glm::vec3(-0.023365f, 0.127052f, 0.549262f) };
    vector<glm::vec3> impulse_beam_right_pts{ glm::vec3(0.023365f, 0.127052f, 0.549262f) };
    auto* impulse_beam_left = new PhaserBeam(_this, map, impulse_beam_left_pts[0], glm::vec3(-0.086847f, 0, 0.363179f), 15.0f, impulse_beam_left_pts);
    auto* impulse_beam_right = new PhaserBeam(_this, map, impulse_beam_right_pts[0], glm::vec3(0.086847f, 0, 0.363179f), 15.0f, impulse_beam_right_pts);
    weapons.addPrimaryWeaponBeam(*impulse_beam_left);
    weapons.addPrimaryWeaponBeam(*impulse_beam_right);

    //saucer phasers
    vector<glm::vec3> top_front_left_pts{ glm::vec3(-0.020005f, 0.157562f, -0.841976f) };
    vector<glm::vec3> top_front_right_pts{ glm::vec3(0.020005f, 0.157562f, -0.841976f) };

    vector<glm::vec3> top_left_left_pts{ glm::vec3(-0.307425f, 0.157562f, -0.518902f) };
    vector<glm::vec3> top_left_right_pts{ glm::vec3(-0.307425f, 0.157562f, -0.553406f) };

    vector<glm::vec3> top_right_left_pts{ glm::vec3(0.307425f, 0.157562f, -0.518902f) };
    vector<glm::vec3> top_right_right_pts{ glm::vec3(0.307425f, 0.157562f, -0.553406f) };

    vector<glm::vec3> btm_front_left_pts{ glm::vec3(0.018112f, 0.032965f, -0.836368f) };
    vector<glm::vec3> btm_front_right_pts{ glm::vec3(-0.018112f, 0.032965f, -0.836368f) };

    vector<glm::vec3> btm_left_left_pts{ glm::vec3(0.303165f, 0.032965f, -0.524129f) };
    vector<glm::vec3> btm_left_right_pts{ glm::vec3(0.303165f, 0.032965f, -0.556734f) };

    vector<glm::vec3> btm_right_left_pts{ glm::vec3(-0.303165f, 0.032965f, -0.524129f) };
    vector<glm::vec3> btm_right_right_pts{ glm::vec3(-0.303165f, 0.032965f, -0.556734f) };

    auto* top_front_beam_left = new PhaserBeam(_this, map, top_front_left_pts[0], glm::vec3(0.0f, 0.065221f, -0.142781f), 35.0f, top_front_left_pts);
    auto* top_front_beam_right = new PhaserBeam(_this, map, top_front_right_pts[0], glm::vec3(0.0f, 0.065221f, -0.142781f), 35.0f, top_front_right_pts);

    auto* top_left_beam_left = new PhaserBeam(_this, map, top_left_left_pts[0], glm::vec3(-0.142781f, 0.065221f, 0.0f), 35.0f, top_left_left_pts);
    auto* top_left_beam_right = new PhaserBeam(_this, map, top_left_right_pts[0], glm::vec3(-0.142781f, 0.065221f, 0.0f), 35.0f, top_left_right_pts);

    auto* top_right_beam_left = new PhaserBeam(_this, map, top_right_left_pts[0], glm::vec3(0.142781f, 0.065221f, 0.0f), 35.0f, top_right_left_pts);
    auto* top_right_beam_right = new PhaserBeam(_this, map, top_right_right_pts[0], glm::vec3(0.142781f, 0.065221f, 0.0f), 35.0f, top_right_right_pts);

    auto* btm_front_beam_left = new PhaserBeam(_this, map, btm_front_left_pts[0], glm::vec3(0.0f, -0.135797f, -0.221065f), 35.0f, btm_front_left_pts);
    auto* btm_front_beam_right = new PhaserBeam(_this, map, btm_front_right_pts[0], glm::vec3(0.0f, -0.135797f, -0.221065f), 35.0f, btm_front_right_pts);

    auto* btm_left_beam_left = new PhaserBeam(_this, map, btm_left_left_pts[0], glm::vec3(-0.221065f, -0.135797f, 0.0f), 35.0f, btm_left_left_pts);
    auto* btm_left_beam_right = new PhaserBeam(_this, map, btm_left_right_pts[0], glm::vec3(-0.221065f, -0.135797f, 0.0f), 35.0f, btm_left_right_pts);

    auto* btm_right_beam_left = new PhaserBeam(_this, map, btm_right_left_pts[0], glm::vec3(0.221065f, -0.135797f, 0.0f), 35.0f, btm_right_left_pts);
    auto* btm_right_beam_right = new PhaserBeam(_this, map, btm_right_right_pts[0], glm::vec3(0.221065f, -0.135797f, 0.0f), 35.0f, btm_right_right_pts);

    weapons.addPrimaryWeaponBeam(*top_front_beam_left);
    weapons.addPrimaryWeaponBeam(*top_front_beam_right);
    weapons.addPrimaryWeaponBeam(*top_left_beam_left);
    weapons.addPrimaryWeaponBeam(*top_left_beam_right);
    weapons.addPrimaryWeaponBeam(*top_right_beam_left);
    weapons.addPrimaryWeaponBeam(*top_right_beam_right);
    weapons.addPrimaryWeaponBeam(*btm_front_beam_left);
    weapons.addPrimaryWeaponBeam(*btm_front_beam_right);
    weapons.addPrimaryWeaponBeam(*btm_left_beam_left);
    weapons.addPrimaryWeaponBeam(*btm_left_beam_right);
    weapons.addPrimaryWeaponBeam(*btm_right_beam_left);
    weapons.addPrimaryWeaponBeam(*btm_right_beam_right);
}
Miranda::~Miranda() {

}
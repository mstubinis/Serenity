#include "Intrepid.h"
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

Intrepid::Intrepid(const AIType::Type ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, "Intrepid", map, ai_type, name, position, scale, collisionType, glm::vec3(0.0f, 0.140914f, -0.585431f)) {

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)        system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)   system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)   system = new ShipSystemYawThrusters(_this);
        else if (i == 3)   system = new ShipSystemRollThrusters(_this);
        else if (i == 4)   system = nullptr; //no cloaking device
        else if (i == 5)   system = new ShipSystemShields(_this, map, 11500.0f, 11500.0f, 11500.0f, 11500.0f, 15000.0f, 15000.0f);
        else if (i == 6)   system = new ShipSystemMainThrusters(_this);
        else if (i == 7)   system = new ShipSystemWarpDrive(_this);
        else if (i == 8)   system = new ShipSystemSensors(_this, map);
        else if (i == 9)   system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 26000.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //torps
    auto* front_right_torp = new PhotonTorpedo(_this, map, glm::vec3(0.199831f, -0.029398f, -0.718548f), glm::vec3(0, 0, -1), 20.0f, 2);
    auto* front_left_torp = new PhotonTorpedo(_this, map, glm::vec3(-0.199831f, -0.029398f, -0.718548f), glm::vec3(0, 0, -1), 20.0f, 2);

    auto* aft_right_torp = new PhotonTorpedo(_this, map, glm::vec3(0.055742f, 0.289654f, 0.338158f), glm::vec3(0, 0, 1), 20.0f, 1);
    auto* aft_left_torp = new PhotonTorpedo(_this, map, glm::vec3(-0.055742f, 0.289654f, 0.338158f), glm::vec3(0, 0, 1), 20.0f, 1);

    weapons.addSecondaryWeaponTorpedo(*front_left_torp);
    weapons.addSecondaryWeaponTorpedo(*front_right_torp);
    weapons.addSecondaryWeaponTorpedo(*aft_left_torp);
    weapons.addSecondaryWeaponTorpedo(*aft_right_torp);

    //phasers
    //top left left
    vector<glm::vec3> top_left_left_pts{
        glm::vec3(-0.589655f, 0.173641f, 0.022045f),
        glm::vec3(-0.566861f, 0.176174f, 0.049904f),
        glm::vec3(-0.540015f, 0.177694f, 0.07827f),
        glm::vec3(-0.514688f, 0.180227f, 0.107649f),
        glm::vec3(-0.48983f, 0.18124f, 0.136995f),
    };
    //top right right
    vector<glm::vec3> top_right_right_pts{
        glm::vec3(0.589655f, 0.173641f, 0.022045f),
        glm::vec3(0.566861f, 0.176174f, 0.049904f),
        glm::vec3(0.540015f, 0.177694f, 0.07827f),
        glm::vec3(0.514688f, 0.180227f, 0.107649f),
        glm::vec3(0.48983f, 0.18124f, 0.136995f),
    };
    auto* top_left_left = new PhaserBeam(_this, map, top_left_left_pts[2], glm::vec3(-0.137638f, 0.171329f, 0.098313f), 40.0f, top_left_left_pts);
    auto* top_right_right = new PhaserBeam(_this, map, top_right_right_pts[2], glm::vec3(0.137638f, 0.171329f, 0.098313f), 40.0f, top_right_right_pts);
    weapons.addPrimaryWeaponBeam(*top_left_left);
    weapons.addPrimaryWeaponBeam(*top_right_right);

    //top front left left
    vector<glm::vec3> top_front_left_left_pts{
        glm::vec3(-0.571704f, 0.307515f, -0.519267f),
        glm::vec3(-0.588341f, 0.307515f, -0.714379f),
        glm::vec3(-0.587385f, 0.307515f, -0.879944f),
        glm::vec3(-0.570191f, 0.307515f, -1.07133f),
        glm::vec3(-0.534068f, 0.307515f, -1.23435f),
    };
    //top front right right
    vector<glm::vec3> top_front_right_right_pts{
        glm::vec3(0.571704f, 0.307515f, -0.519267f),
        glm::vec3(0.588341f, 0.307515f, -0.714379f),
        glm::vec3(0.587385f, 0.307515f, -0.879944f),
        glm::vec3(0.570191f, 0.307515f, -1.07133f),
        glm::vec3(0.534068f, 0.307515f, -1.23435f),
    };
    auto* top_front_left_left = new PhaserBeam(_this, map, top_front_left_left_pts[2], glm::vec3(-0.18991f, 0.137347f, -0.011459f), 55.0f, top_front_left_left_pts);
    auto* top_front_right_right = new PhaserBeam(_this, map, top_front_right_right_pts[2], glm::vec3(0.18991f, 0.137347f, -0.011459f), 55.0f, top_front_right_right_pts);
    weapons.addPrimaryWeaponBeam(*top_front_left_left);
    weapons.addPrimaryWeaponBeam(*top_front_right_right);

    //top front left
    vector<glm::vec3> top_front_left_pts{
        glm::vec3(-0.534068f, 0.307515f, -1.23435f),
        glm::vec3(-0.459423f, 0.307515f, -1.45954f),
        glm::vec3(-0.384151f, 0.307515f, -1.62514f),
        glm::vec3(-0.271244f, 0.307515f, -1.79826f),
        glm::vec3(-0.154572f, 0.307515f, -1.89611f),
    };
    //top front right
    vector<glm::vec3> top_front_right_pts{
        glm::vec3(0.534068f, 0.307515f, -1.23435f),
        glm::vec3(0.459423f, 0.307515f, -1.45954f),
        glm::vec3(0.384151f, 0.307515f, -1.62514f),
        glm::vec3(0.271244f, 0.307515f, -1.79826f),
        glm::vec3(0.154572f, 0.307515f, -1.89611f),
    };
    auto* top_front_left = new PhaserBeam(_this, map, top_front_left_pts[2], glm::vec3(-0.146001f, 0.137347f, -0.177685f), 55.0f, top_front_left_pts);
    auto* top_front_right = new PhaserBeam(_this, map, top_front_right_pts[2], glm::vec3(0.146001f, 0.137347f, -0.177685f), 55.0f, top_front_right_pts);
    weapons.addPrimaryWeaponBeam(*top_front_left);
    weapons.addPrimaryWeaponBeam(*top_front_right);

    //ventral center
    vector<glm::vec3> ventral_center_pts{
        glm::vec3(-0.265627f, -0.326306f, -0.161425f),
        glm::vec3(-0.132509f, -0.377703f, -0.161425f),
        glm::vec3(0.0f, -0.393f, -0.161425f),
        glm::vec3(0.132509f, -0.377703f, -0.161425f),
        glm::vec3(0.265627f, -0.326306f, -0.161425f),
    };
    auto* ventral_center = new PhaserBeam(_this, map, ventral_center_pts[2], glm::vec3(0.0f, -1.0f, 0.0f), 40.0f, ventral_center_pts);
    weapons.addPrimaryWeaponBeam(*ventral_center);

    //ventral nacelle left
    vector<glm::vec3> ventral_nacelle_left_pts{
        glm::vec3(0.516639f, -0.175676f, 1.43193f),
        glm::vec3(0.516639f, -0.175676f, 1.46779f),
        glm::vec3(0.516639f, -0.175676f, 1.50183f),
        glm::vec3(0.516639f, -0.175676f, 1.5383f),
        glm::vec3(0.516639f, -0.175676f, 1.57416f),
    };
    //ventral nacelle right
    vector<glm::vec3> ventral_nacelle_right_pts{
        glm::vec3(-0.516639f, -0.175676f, 1.43193f),
        glm::vec3(-0.516639f, -0.175676f, 1.46779f),
        glm::vec3(-0.516639f, -0.175676f, 1.50183f),
        glm::vec3(-0.516639f, -0.175676f, 1.5383f),
        glm::vec3(-0.516639f, -0.175676f, 1.57416f),
    };
    auto* ventral_nacelle_left = new PhaserBeam(_this, map, ventral_nacelle_left_pts[2], glm::vec3(0.037638f, -0.162941f, 0.045366f), 35.0f, ventral_nacelle_left_pts);
    auto* ventral_nacelle_right = new PhaserBeam(_this, map, ventral_nacelle_right_pts[2], glm::vec3(-0.037638f, -0.162941f, 0.045366f), 35.0f, ventral_nacelle_right_pts);

    weapons.addPrimaryWeaponBeam(*ventral_nacelle_left);
    weapons.addPrimaryWeaponBeam(*ventral_nacelle_right);

    //ventral btm left
    vector<glm::vec3> ventral_btm_left_pts{
        glm::vec3(0.180012f, -0.175676f, 2.05634f),
        glm::vec3(0.154803f, -0.175676f, 2.06748f),
        glm::vec3(0.13018f, -0.175676f, 2.0751f),
        glm::vec3(0.103505f, -0.175676f, 2.08125f),
        glm::vec3(0.080054f, -0.175676f, 2.08506f),
    };
    //ventral btm right
    vector<glm::vec3> ventral_btm_right_pts{
        glm::vec3(-0.180012f, -0.175676f, 2.05634f),
        glm::vec3(-0.154803f, -0.175676f, 2.06748f),
        glm::vec3(-0.13018f, -0.175676f, 2.0751f),
        glm::vec3(-0.103505f, -0.175676f, 2.08125f),
        glm::vec3(-0.080054f, -0.175676f, 2.08506f),
    };
    auto* ventral_btm_left = new PhaserBeam(_this, map, ventral_btm_left_pts[2], glm::vec3(0.038688f, -0.162941f, 0.127294f), 40.0f, ventral_btm_left_pts);
    auto* ventral_btm_right = new PhaserBeam(_this, map, ventral_btm_right_pts[2], glm::vec3(-0.038688f, -0.162941f, 0.127294f), 40.0f, ventral_btm_right_pts);

    weapons.addPrimaryWeaponBeam(*ventral_btm_left);
    weapons.addPrimaryWeaponBeam(*ventral_btm_right);

    //ventral left left
    vector<glm::vec3> ventral_left_left_pts{
        glm::vec3(0.599802f, 0.03295f, -0.00685f),
        glm::vec3(0.616511f, 0.03295f, -0.030301f),
        glm::vec3(0.633512f, 0.03295f, -0.053752f),
        glm::vec3(0.649928f, 0.03295f, -0.076909f),
        glm::vec3(0.665464f, 0.03295f, -0.100653f),
    };
    //ventral right right
    vector<glm::vec3> ventral_right_right_pts{
        glm::vec3(-0.599802f, 0.03295f, -0.00685f),
        glm::vec3(-0.616511f, 0.03295f, -0.030301f),
        glm::vec3(-0.633512f, 0.03295f, -0.053752f),
        glm::vec3(-0.649928f, 0.03295f, -0.076909f),
        glm::vec3(-0.665464f, 0.03295f, -0.100653f),
    };
    auto* ventral_left_left = new PhaserBeam(_this, map, ventral_left_left_pts[2], glm::vec3(0.10652f, -0.163131f, 0.12681f), 40.0f, ventral_left_left_pts);
    auto* ventral_right_right = new PhaserBeam(_this, map, ventral_right_right_pts[2], glm::vec3(-0.10652f, -0.163131f, 0.12681f), 40.0f, ventral_right_right_pts);

    weapons.addPrimaryWeaponBeam(*ventral_left_left);
    weapons.addPrimaryWeaponBeam(*ventral_right_right);

    //ventral top left left
    vector<glm::vec3> ventral_top_left_left_pts{
        glm::vec3(0.802006f, 0.031296f, -0.767329f),
        glm::vec3(0.793844f, 0.031296f, -0.968032f),
        glm::vec3(0.767708f, 0.031296f, -1.18801f),
        glm::vec3(0.732859f, 0.031296f, -1.39928f),
        glm::vec3(0.684943f, 0.031296f, -1.62579f),
    };
    //ventral top right right
    vector<glm::vec3> ventral_top_right_right_pts{
        glm::vec3(-0.802006f, 0.031296f, -0.767329f),
        glm::vec3(-0.793844f, 0.031296f, -0.968032f),
        glm::vec3(-0.767708f, 0.031296f, -1.18801f),
        glm::vec3(-0.732859f, 0.031296f, -1.39928f),
        glm::vec3(-0.684943f, 0.031296f, -1.62579f),
    };
    //ventral top left
    vector<glm::vec3> ventral_top_left_pts{
        glm::vec3(0.684943f, 0.031296f, -1.62579f),
        glm::vec3(0.60218f, 0.031296f, -1.84998f),
        glm::vec3(0.486653f, 0.031296f, -2.08088f),
        glm::vec3(0.34334f, 0.031296f, -2.27714f),
        glm::vec3(0.152503f, 0.031296f, -2.44268f),
    };
    //ventral top right
    vector<glm::vec3> ventral_top_right_pts{
        glm::vec3(-0.684943f, 0.031296f, -1.62579f),
        glm::vec3(-0.60218f, 0.031296f, -1.84998f),
        glm::vec3(-0.486653f, 0.031296f, -2.08088f),
        glm::vec3(-0.34334f, 0.031296f, -2.27714f),
        glm::vec3(-0.152503f, 0.031296f, -2.44268f),
    };
    auto* ventral_top_left_left = new PhaserBeam(_this, map, ventral_top_left_left_pts[2], glm::vec3(0.267854f, -0.348184f, -0.044608f), 55.0f, ventral_top_left_left_pts);
    auto* ventral_top_right_right = new PhaserBeam(_this, map, ventral_top_right_right_pts[2], glm::vec3(-0.267854f, -0.348184f, -0.044608f), 55.0f, ventral_top_right_right_pts);
    auto* ventral_top_left = new PhaserBeam(_this, map, ventral_top_left_pts[2], glm::vec3(0.198403f, -0.348184f, -0.185397f), 55.0f, ventral_top_left_pts);
    auto* ventral_top_right = new PhaserBeam(_this, map, ventral_top_right_pts[2], glm::vec3(-0.198403f, -0.348184f, -0.185397f), 55.0f, ventral_top_right_pts);

    weapons.addPrimaryWeaponBeam(*ventral_top_left_left);
    weapons.addPrimaryWeaponBeam(*ventral_top_right_right);
    weapons.addPrimaryWeaponBeam(*ventral_top_left);
    weapons.addPrimaryWeaponBeam(*ventral_top_right);


    vector<glm::vec3> hull_target_points = {
        glm::vec3(0,0.187934f, -2.14433f),
        glm::vec3(0,0.187934f, -1.76738f),
        glm::vec3(0,0.187934f, -1.42096f),
        glm::vec3(0,0.187934f, -1.17599f),
        glm::vec3(0,0.263242f, -0.882292f),
        glm::vec3(0,0.263242f, -0.573299f),
        glm::vec3(0,0.263242f, -0.215486f),
        glm::vec3(0,0.082578f, 0.2588f),
        glm::vec3(0,0.0487f, 0.506052f),
        glm::vec3(0,-0.03808f, 0.836647f),
        glm::vec3(0,-0.03808f, 1.28013f),
        glm::vec3(0,-0.03808f, 1.66896f),
        glm::vec3(0,-0.239327f, 0.570865f),
        glm::vec3(0,-0.280726f, -0.076472f),
        glm::vec3(-0.816978f,-0.121007f, 0.811069f),
        glm::vec3(0.816978f,-0.121007f, 0.811069f),
        glm::vec3(-0.816978f,-0.121007f, 1.3022f),
        glm::vec3(0.816978f,-0.121007f, 1.3022f),
        glm::vec3(-0.816978f,-0.121007f, 1.79231f),
        glm::vec3(0.816978f,-0.121007f, 1.79231f),
        glm::vec3(-0.545999f,-0.121007f,1.30003f),
        glm::vec3(0.545999f,-0.121007f, 1.30003f),
        glm::vec3(-0.293086f,-0.121007f,1.58907f),
        glm::vec3(0.293086f,-0.121007f, 1.58907f),
        glm::vec3(-0.258588f,0.146209f,-2.0831f),
        glm::vec3(0.258588f,0.146209f, -2.0831f),
        glm::vec3(-0.540406f,0.146209f,-1.66037f),
        glm::vec3(0.540406f,0.146209f, -1.66037f),
        glm::vec3(-0.615326f,0.146209f,-1.07974f),
        glm::vec3(0.615326f,0.146209f, -1.07974f),
        glm::vec3(-0.540406f,0.146209f,-0.424189f),
        glm::vec3(0.540406f,0.146209f, -0.424189f),
        glm::vec3(0.251651f,0.146209f,0.098691f),
        glm::vec3(-0.251651f,0.146209f, 0.098691f),
        glm::vec3(-0.279746f,0.202415f,-1.54019f),
        glm::vec3(0.279746f,0.202415f, -1.54019f),
        glm::vec3(-0.354666f,0.202415f,-1.02511f),
        glm::vec3(0.354666f,0.202415f, -1.02511f),
        glm::vec3(-0.195461f,0.202415f,-0.257179f),
        glm::vec3(0.195461f,0.202415f, -0.257179f),
        glm::vec3(-0.128033f,-0.005906f,0.664339f),
        glm::vec3(0.128033f,-0.005906f, 0.664339f),
        glm::vec3(-0.173718f,0.435077f,-0.557595f),
        glm::vec3(0.173718f,0.435077f, -0.557595f),
    };
    addHullTargetPoints(hull_target_points);
}
Intrepid::~Intrepid() {

}

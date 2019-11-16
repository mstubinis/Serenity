#include "Defiant.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PulsePhaser.h"
#include "../../weapons/PhaserBeam.h"
#include "../../weapons/QuantumTorpedo.h"
#include "../../weapons/PhotonTorpedo.h"

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

Defiant::Defiant(const AIType::Type ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client,"Defiant",map,ai_type,name,position,scale, collisionType){

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this);
        else if (i == 4)  system = nullptr; //no cloaking device
        //else if (i == 4)  system = new ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = new ShipSystemShields(_this, map, 9000.0f, 9000.0f, 9000.0f, 9000.0f, 12000.0f, 12000.0f, glm::vec3(0.0f), glm::vec3(1.15f, 2.1f, 1.15f));
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 17000.0f);
        m_ShipSystems.emplace(i, system);
    }

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //blender 3d to game 3d: switch y and z, then negate z
    auto* leftTop     = new PulsePhaser(_this, map, glm::vec3(-0.573355f, 0.072793f, -0.207088f), glm::vec3(0.0055f, 0, -1), 10.0f, 6,250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);
    auto* leftBottom  = new PulsePhaser(_this, map, glm::vec3(-0.434018f, -0.163775, -0.093399), glm::vec3(0.005f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);
    auto* rightBottom = new PulsePhaser(_this, map, glm::vec3(0.434018f, -0.163775, -0.093399), glm::vec3(-0.005f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);
    auto* rightTop    = new PulsePhaser(_this, map, glm::vec3(0.573355f, 0.072793f, -0.207088f), glm::vec3(-0.0055f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);

    auto* leftTorp = new QuantumTorpedo(_this, map, glm::vec3(-0.358887f, 0.023574f, -0.657542f), glm::vec3(0, 0, -1), 15.0f);
    auto* rightTorp = new QuantumTorpedo(_this, map, glm::vec3(0.358887f, 0.023574f, -0.657542f), glm::vec3(0, 0, -1), 15.0f);
    auto* aftTorp = new PhotonTorpedo(_this, map, glm::vec3(0.0f, 0.019991f, 1.04274f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addPrimaryWeaponCannon(*leftTop, true);
    weapons.addPrimaryWeaponCannon(*leftBottom, true);
    weapons.addPrimaryWeaponCannon(*rightBottom, true);
    weapons.addPrimaryWeaponCannon(*rightTop, true);

    weapons.addSecondaryWeaponTorpedo(*leftTorp, true);
    weapons.addSecondaryWeaponTorpedo(*rightTorp, true);
    weapons.addSecondaryWeaponTorpedo(*aftTorp);

    //phasers

    vector<glm::vec3> top_beam_pts{
        glm::vec3(-0.095116f, 0.150892f, -0.010849f),
        glm::vec3(-0.050611f, 0.150892f, 0.017642f),
        glm::vec3(0.0f, 0.150892f, 0.026531f),
        glm::vec3(0.050611f, 0.150892f, 0.017642f),
        glm::vec3(0.095116f, 0.150892f, -0.010849f),
    };
    vector<glm::vec3> ventral_left_beam_pts{
        glm::vec3(-0.782141f, -0.087931f, 0.135678f),
        glm::vec3(-0.735904f, -0.113648f, 0.135678f),
        glm::vec3(-0.685293f, -0.129982f, 0.135678f),
        glm::vec3(-0.633076f, -0.144912f, 0.135678f),
        glm::vec3(-0.58733f, -0.157489f, 0.135678f),
    };
    vector<glm::vec3> ventral_right_beam_pts{
        glm::vec3(0.782141f, -0.087931f, 0.135678f),
        glm::vec3(0.735904f, -0.113648f, 0.135678f),
        glm::vec3(0.685293f, -0.129982f, 0.135678f),
        glm::vec3(0.633076f, -0.144912f, 0.135678f),
        glm::vec3(0.58733f, -0.157489f, 0.135678f),
    };
    auto* topBeam = new PhaserBeam(_this, map, glm::vec3(0.0f, 0.150892f, 0.026531f), glm::vec3(0.0f, 1.0f, 0.0f), 65.0f, top_beam_pts);
    auto* ventralLeftBeam = new PhaserBeam(_this, map, glm::vec3(-0.685293f, -0.129982f, 0.135678f), glm::vec3(-0.056611f, -0.228363f, 0.0f), 70.0f, ventral_left_beam_pts);
    auto* ventralRightBeam = new PhaserBeam(_this, map, glm::vec3(0.685293f, -0.129982f, 0.135678f), glm::vec3(0.056611f, -0.228363f, 0.0f), 70.0f, ventral_right_beam_pts);

    weapons.addPrimaryWeaponBeam(*topBeam);
    weapons.addPrimaryWeaponBeam(*ventralRightBeam);
    weapons.addPrimaryWeaponBeam(*ventralLeftBeam);
    
    vector<glm::vec3> hull_target_points = {
        glm::vec3(0.001f,0.001f,-0.944632f),
        glm::vec3(0.001f, 0.024758f, -0.671542f),
        glm::vec3(-0.237229f, 0.024758f, -0.444627f),
        glm::vec3(0.237229f, 0.024758f, -0.444627f),
        glm::vec3(-0.260109f, 0.024758f, -0.012753f),
        glm::vec3(0.260109f, 0.024758f, -0.012753f),
        glm::vec3(-0.499616f, -0.018225f, 0.2071f),
        glm::vec3(0.499616f, -0.018225f, 0.2071f),
        glm::vec3(-0.188694f, 0.0429f, 0.404107f),
        glm::vec3(0.188694f, 0.0429f, 0.404107f),
        glm::vec3(-0.188694f, 0.0429f, 0.750788f),
        glm::vec3(0.188694f, 0.0429f, 0.750788f),
        glm::vec3(0.0f, 0.026626f, 0.870882f),
        glm::vec3(0.0f, 0.026626f, 0.443375f),
        glm::vec3(0.0f, 0.026626f, 0.136259f),
        glm::vec3(0.0f, 0.026626f, -0.23),
        glm::vec3(0.0f, 0.026626f, -0.46),
        glm::vec3(-0.118821f,0.026626f, -0.201665f),
        glm::vec3(0.118821f,0.026626f, -0.201665f),
        glm::vec3(-0.714321f,-0.010267f,-0.013934f),
        glm::vec3(0.714321f,-0.010267f,-0.013934f),
        glm::vec3(-0.697608f,-0.010267f,0.401503f),
        glm::vec3(0.697608f,-0.010267f,0.401503f),
        glm::vec3(-0.571067f,-0.010267f,0.279737f),
        glm::vec3(0.571067f,-0.010267f,0.279737f),
        glm::vec3(-0.633144f,-0.010267f,0.055305f),
        glm::vec3(0.633144f,-0.010267f,0.055305f),
        glm::vec3(-0.458851f,-0.010267f,-0.076011f),
        glm::vec3(0.458851f,-0.010267f,-0.076011f),
        glm::vec3(-0.110266f,0.070924f,0.203335f),
        glm::vec3(0.110266f,0.070924f,0.203335f),
        glm::vec3(-0.09021f,0.070924f,0.498439f),
        glm::vec3(0.09021f,0.070924f,0.498439f),
        glm::vec3(-0.164702f,0.030813f,0.87663f),
        glm::vec3(0.164702f,0.030813f,0.87663f),
    };
    addHullTargetPoints(hull_target_points);

    m_AI->installFireAtWill(_this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
}
Defiant::~Defiant() {

}
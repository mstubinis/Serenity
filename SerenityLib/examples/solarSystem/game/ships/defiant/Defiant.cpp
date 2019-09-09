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

using namespace std;

Defiant::Defiant(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client,ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,"Defiant",map,player,name,position,scale, collisionType){

    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(*this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
        else if (i == 2)  system = new ShipSystemYawThrusters(*this);
        else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = nullptr; //no cloaking device
        //else if (i == 4)  system = new ShipSystemCloakingDevice(*this);
        else if (i == 5)  system = new ShipSystemShields(*this, map, 9000);
        else if (i == 6)  system = new ShipSystemMainThrusters(*this);
        else if (i == 7)  system = new ShipSystemWarpDrive(*this);
        else if (i == 8)  system = new ShipSystemSensors(*this, map);
        else if (i == 9)  system = new ShipSystemWeapons(*this);
        else if (i == 10)  system = new ShipSystemHull(*this, map, 8000);
        m_ShipSystems.emplace(i, system);
    }

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //blender 3d to game 3d: switch y and z, then negate z
    auto* leftTop     = new PulsePhaser(*this, map, glm::vec3(-0.573355f, 0.072793f, -0.207088f), glm::vec3(0.0055f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);
    auto* leftBottom  = new PulsePhaser(*this, map, glm::vec3(-0.434018f, -0.163775, -0.093399), glm::vec3(0.005f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);
    auto* rightBottom = new PulsePhaser(*this, map, glm::vec3(0.434018f, -0.163775, -0.093399), glm::vec3(-0.005f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);
    auto* rightTop    = new PulsePhaser(*this, map, glm::vec3(0.573355f, 0.072793f, -0.207088f), glm::vec3(-0.0055f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f);

    auto* leftTorp = new QuantumTorpedo(*this, map, glm::vec3(-0.358887f, 0.023574f, -0.657542f), glm::vec3(0, 0, -1), 15.0f);
    auto* rightTorp = new QuantumTorpedo(*this, map, glm::vec3(0.358887f, 0.023574f, -0.657542f), glm::vec3(0, 0, -1), 15.0f);
    auto* aftTorp = new PhotonTorpedo(*this, map, glm::vec3(0.0f, 0.019991f, 1.04274f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addPrimaryWeaponCannon(*leftTop);
    weapons.addPrimaryWeaponCannon(*leftBottom);
    weapons.addPrimaryWeaponCannon(*rightBottom);
    weapons.addPrimaryWeaponCannon(*rightTop);

    weapons.addSecondaryWeaponTorpedo(*leftTorp);
    weapons.addSecondaryWeaponTorpedo(*rightTorp);
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
    auto* topBeam = new PhaserBeam(*this, map, glm::vec3(0.0f, 0.150892f, 0.026531f), glm::vec3(0.0f, 1.0f, 0.0f), 65.0f, top_beam_pts);
    auto* ventralLeftBeam = new PhaserBeam(*this, map, glm::vec3(-0.685293f, -0.129982f, 0.135678f), glm::vec3(-0.056611f, -0.228363f, 0.0f), 70.0f, ventral_left_beam_pts);
    auto* ventralRightBeam = new PhaserBeam(*this, map, glm::vec3(0.685293f, -0.129982f, 0.135678f), glm::vec3(0.056611f, -0.228363f, 0.0f), 70.0f, ventral_right_beam_pts);

    weapons.addPrimaryWeaponBeam(*topBeam);
    weapons.addPrimaryWeaponBeam(*ventralRightBeam);
    weapons.addPrimaryWeaponBeam(*ventralLeftBeam);
}
Defiant::~Defiant() {

}
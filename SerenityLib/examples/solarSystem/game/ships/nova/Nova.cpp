#include "Nova.h"
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

Nova::Nova(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client, ResourceManifest::NovaMesh, ResourceManifest::NovaMaterial, "Nova", map, player, name, position, scale, collisionType, glm::vec3(0.0f, -0.136252f, -0.420117f)) {

    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(*this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
        else if (i == 2)  system = new ShipSystemYawThrusters(*this);
        else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = nullptr; //no cloaking device
        else if (i == 5)  system = new ShipSystemShields(*this, map, 10500.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(*this);
        else if (i == 7)  system = new ShipSystemWarpDrive(*this);
        else if (i == 8)  system = new ShipSystemSensors(*this, map);
        else if (i == 9)  system = new ShipSystemWeapons(*this);
        else if (i == 10)  system = new ShipSystemHull(*this, map, 8500.0f);
        m_ShipSystems.emplace(i, system);
    }
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTorp = new PhotonTorpedo(*this, map, glm::vec3(-0.223032f, -0.089813f, -1.82296f), glm::vec3(0, 0, -1), 15.0f);
    auto* rightTorp = new PhotonTorpedo(*this, map, glm::vec3(0.223032f, -0.089813f, -1.82296f), glm::vec3(0, 0, -1), 15.0f);
    auto* aftTorp = new PhotonTorpedo(*this, map, glm::vec3(0.0f, -0.182921f, 0.810231f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addSecondaryWeaponTorpedo(*leftTorp);
    weapons.addSecondaryWeaponTorpedo(*rightTorp);
    weapons.addSecondaryWeaponTorpedo(*aftTorp);

    //phasers

    //top-left beam
    vector<glm::vec3> top_left_beam_pts{ 
        glm::vec3(-0.611083f, -0.006686f, -0.966413f),
        glm::vec3(-0.577333f, -0.004358f, -1.10134f),
        glm::vec3(-0.52394f, 0.000598f, -1.23735f), 
        glm::vec3(-0.437519f, 0.00164f, -1.38157f),
        glm::vec3(-0.359835f, 0.001445f, -1.49756f),
    };
    //top-right beam
    vector<glm::vec3> top_right_beam_pts{ 
        glm::vec3(0.611083f, -0.006686f, -0.966413f),
        glm::vec3(0.577333f, -0.004358f, -1.10134f),
        glm::vec3(0.52394f, 0.000598f, -1.23735f),
        glm::vec3(0.437519f, 0.00164f, -1.38157f),
        glm::vec3(0.359835f, 0.001445f, -1.49756f),
    };
    //bottom-left beam
    vector<glm::vec3> bottom_left_beam_pts{ 
        glm::vec3(-0.715631f, -0.161622f, -0.99928f),
        glm::vec3(-0.642443f, -0.161622f, -1.18936f),
        glm::vec3(-0.551941f, -0.161622f, -1.36694f),
        glm::vec3(-0.443947f, -0.161622f, -1.54072f),
        glm::vec3(-0.326052f, -0.161622f, -1.68563f),
    };
    //bottom-right beam
    vector<glm::vec3> bottom_right_beam_pts{ 
        glm::vec3(0.715631f, -0.161622f, -0.99928f),
        glm::vec3(0.642443f, -0.161622f, -1.18936f),
        glm::vec3(0.551941f, -0.161622f, -1.36694f),
        glm::vec3(0.443947f, -0.161622f, -1.54072f),
        glm::vec3(0.326052f, -0.161622f, -1.68563f),
    };
    //ventral center beam
    vector<glm::vec3> ventral_center_beam_pts{ 
        glm::vec3(-0.158448f, -0.385f, -0.415398f),
        glm::vec3(-0.079593f, -0.40085f, -0.415398f),
        glm::vec3(0.0f, -0.411908f, -0.415398f),
        glm::vec3(0.079593f, -0.40085f, -0.415398f),
        glm::vec3(0.158448f, -0.385f, -0.415398f),
    };
    //bottom-left-aft beam
    vector<glm::vec3> bottom_left_aft_beam_pts{ 
        glm::vec3(-0.566489f, -0.158103f, -0.512599f),
        glm::vec3(-0.540047f, -0.158103f, -0.495966f),
        glm::vec3(-0.511899f, -0.158103f, -0.478266f),
        glm::vec3(-0.482685f, -0.158103f, -0.459714f),
        glm::vec3(-0.454963f, -0.158103f, -0.442655f),
    };
    //bottom-right-aft beam
    vector<glm::vec3> bottom_right_aft_beam_pts{ 
        glm::vec3(0.566489f, -0.158103f, -0.512599f),
        glm::vec3(0.540047f, -0.158103f, -0.495966f),
        glm::vec3(0.511899f, -0.158103f, -0.478266f),
        glm::vec3(0.482685f, -0.158103f, -0.459714f),
        glm::vec3(0.454963f, -0.158103f, -0.442655f),
    };
    //top-left aft beam
    vector<glm::vec3> top_left_aft_beam_pts{ 
        glm::vec3(-0.643016f, -0.106705f, -0.50182f),
        glm::vec3(-0.614911f, -0.106705f, -0.482998f),
        glm::vec3(-0.583796f, -0.106705f, -0.464685f),
        glm::vec3(-0.552619f, -0.106705f, -0.4458f),
        glm::vec3(-0.523737f, -0.106705f, -0.428117f),
    };
    //top-right aft beam
    vector<glm::vec3> top_right_aft_beam_pts{ 
        glm::vec3(0.643016f, -0.106705f, -0.50182f),
        glm::vec3(0.614911f, -0.106705f, -0.482998f),
        glm::vec3(0.583796f, -0.106705f, -0.464685f),
        glm::vec3(0.552619f, -0.106705f, -0.4458f),
        glm::vec3(0.523737f, -0.106705f, -0.428117f),
    };
    //top-left aft aft beam
    vector<glm::vec3> top_left_aft_aft_beam_pts{
        glm::vec3(-0.168087f, -0.102565f, 0.662059f),
        glm::vec3(-0.160836f, -0.101073f, 0.688502f),
        glm::vec3(-0.153586f, -0.09958f, 0.719422f),
        glm::vec3(-0.147829f, -0.098087f, 0.74757f),
        glm::vec3(-0.140578f, -0.097021f, 0.775505f),
    };
    //top-right aft aft beam
    vector<glm::vec3> top_right_aft_aft_beam_pts{
        glm::vec3(0.168087f, -0.102565f, 0.662059f),
        glm::vec3(0.160836f, -0.101073f, 0.688502f),
        glm::vec3(0.153586f, -0.09958f, 0.719422f),
        glm::vec3(0.147829f, -0.098087f, 0.74757f),
        glm::vec3(0.140578f, -0.097021f, 0.775505f),
    };

    auto* topLeftBeam = new PhaserBeam(*this, map, glm::vec3(-0.52394f, 0.000598f, -1.23735f), glm::vec3(-0.170995f, 0.307375f, -0.083132f), 80.0f, top_left_beam_pts);
    auto* topRightBeam = new PhaserBeam(*this, map, glm::vec3(0.52394f, 0.000598f, -1.23735f), glm::vec3(0.170995f, 0.307375f, -0.083132f), 80.0f, top_right_beam_pts);

    auto* bottomLeftBeam = new PhaserBeam(*this, map, glm::vec3(-0.56826f, -0.165129f, -1.338f), glm::vec3(-0.097147f, -0.269223f, -0.055331f), 80.0f, bottom_left_beam_pts);
    auto* bottomRightBeam = new PhaserBeam(*this, map, glm::vec3(0.56826f, -0.165129f, -1.338f), glm::vec3(0.097147f, -0.269223f, -0.055331f), 80.0f, bottom_right_beam_pts);

    auto* ventralCenterBeam = new PhaserBeam(*this, map, glm::vec3(0.0f, -0.411908f, -0.415398f), glm::vec3(0.0f, -1.0f, 0.0f), 70.0f, ventral_center_beam_pts);

    auto* bottomLeftAftBeam = new PhaserBeam(*this, map, glm::vec3(0.0f, -0.411908f, -0.415398f), glm::vec3(-0.085868f, -0.421631f, 0.140047f), 40.0f, bottom_left_aft_beam_pts);
    auto* bottomRightAftBeam = new PhaserBeam(*this, map, glm::vec3(0.0f, -0.411908f, -0.415398f), glm::vec3(0.085868f, -0.421631f, 0.140047f), 40.0f, bottom_right_aft_beam_pts);

    auto* topLeftAftBeam = new PhaserBeam(*this, map, glm::vec3(-0.583796f, -0.106705f, -0.464685f), glm::vec3(-0.066311f, 0.17146f, 0.103438f), 15.0f, top_left_aft_beam_pts);
    auto* topRightAftBeam = new PhaserBeam(*this, map, glm::vec3(0.583796f, -0.106705f, -0.464685f), glm::vec3(0.066311f, 0.17146f, 0.103438f), 15.0f, top_right_aft_beam_pts);

    auto* topLeftAftAftBeam = new PhaserBeam(*this, map, glm::vec3(-0.153586f, -0.09958f, 0.719422f), glm::vec3(-0.127185f, 0.257314f, 0.031746f), 45.0f, top_left_aft_aft_beam_pts);
    auto* topRightAftAftBeam = new PhaserBeam(*this, map, glm::vec3(0.153586f, -0.09958f, 0.719422f), glm::vec3(0.127185f, 0.257314f, 0.031746f), 45.0f, top_right_aft_aft_beam_pts);

    weapons.addPrimaryWeaponBeam(*topLeftBeam);
    weapons.addPrimaryWeaponBeam(*topRightBeam);
    weapons.addPrimaryWeaponBeam(*bottomLeftBeam);
    weapons.addPrimaryWeaponBeam(*bottomRightBeam);
    weapons.addPrimaryWeaponBeam(*ventralCenterBeam);

    weapons.addPrimaryWeaponBeam(*bottomLeftAftBeam);
    weapons.addPrimaryWeaponBeam(*bottomRightAftBeam);

    weapons.addPrimaryWeaponBeam(*topLeftAftBeam);
    weapons.addPrimaryWeaponBeam(*topRightAftBeam);

    weapons.addPrimaryWeaponBeam(*topLeftAftAftBeam);
    weapons.addPrimaryWeaponBeam(*topRightAftAftBeam);
}
Nova::~Nova() {

}
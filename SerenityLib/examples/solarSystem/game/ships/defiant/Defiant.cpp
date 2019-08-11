#include "Defiant.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/PulsePhaser.h"

using namespace std;

Defiant::Defiant(Client& client, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType, Map* map)
:Ship(client,ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,"Defiant",player,name,position,scale, collisionType,map){

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    PulsePhaser* leftTop = new PulsePhaser(*this, *map, glm::vec3(-0.573355f, 0.07134f, 0.072925f), glm::vec3(0, 0, -1), 10.0f);
    PulsePhaser* leftBottom = new PulsePhaser(*this, *map, glm::vec3(-0.434018f, -0.1592f, 0.163752f), glm::vec3(0, 0, -1), 10.0f);
    PulsePhaser* rightBottom = new PulsePhaser(*this, *map, glm::vec3(0.434018f, -0.1592f, 0.163752f), glm::vec3(0, 0, -1), 10.0f);
    PulsePhaser* rightTop = new PulsePhaser(*this, *map, glm::vec3(0.573355f, 0.07134f, 0.072925f), glm::vec3(0, 0, -1), 10.0f);

    //front left torpedo  (-0.358887, 0.657542, 0.023574)
    //front right torpedo (0.358887, 0.657542, 0.023574)

    weapons.addPrimaryWeaponCannon(*leftTop);
    weapons.addPrimaryWeaponCannon(*leftBottom);
    weapons.addPrimaryWeaponCannon(*rightBottom);
    weapons.addPrimaryWeaponCannon(*rightTop);
}
Defiant::~Defiant() {

}
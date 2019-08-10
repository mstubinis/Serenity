#include "Defiant.h"
#include "../../ResourceManifest.h"

using namespace std;

Defiant::Defiant(Client& client, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType, Map* map)
:Ship(client,ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,"Defiant",player,name,position,scale, collisionType,map){


}
Defiant::~Defiant() {

}
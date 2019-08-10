#include "Brel.h"
#include "../../ResourceManifest.h"

using namespace std;

Brel::Brel(Client& client, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType, Map* map)
:Ship(client, ResourceManifest::BrelMesh, ResourceManifest::BrelMaterial, "Brel", player, name, position, scale, collisionType, map) {


}
Brel::~Brel() {

}
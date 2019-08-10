#include "Shrike.h"
#include "../../ResourceManifest.h"

using namespace std;

Shrike::Shrike(Client& client, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType, Map* map)
:Ship(client, ResourceManifest::ShrikeMesh, ResourceManifest::ShrikeMaterial, "Shrike", player, name, position, scale, collisionType, map) {


}
Shrike::~Shrike() {

}
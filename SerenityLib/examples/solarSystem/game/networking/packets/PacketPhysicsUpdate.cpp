#include "PacketPhysicsUpdate.h"

#include "../../Ship.h"
#include "../../map/Anchor.h"
#include "../../map/Map.h"
#include <core/engine/math/Engine_Math.h>

using namespace std;
using namespace Engine;

PacketPhysicsUpdate::PacketPhysicsUpdate() {
    qXYZ = lx = ly = lz = ax = ay = az = 0;
    px = py = pz = wx = wy = wz = 0.0f;
    qw = 1;
    team_number = ai_type = 0;
    ship_class = ship_map_key = data = "";
}
PacketPhysicsUpdate::PacketPhysicsUpdate(Ship& ship, Map& map, Anchor* finalAnchor, const vector<string>& anchorList, const string& username) : PacketPhysicsUpdate() {
    auto& ship_entity = ship.entity();
    EntityDataRequest request(ship_entity);
    const auto pbody  = ship_entity.getComponent<ComponentBody>(request);

    ship_class             = ship.getClass();
    ship_map_key           = ship.getMapKey();
    player_username        = username;
    team_number            = static_cast<unsigned int>(ship.getTeam().getTeamNumber());
    ai_type                = static_cast<unsigned int>(ship.getAIType());
    if (pbody) {
        auto& body         = *pbody;

        const auto pos     = body.position();
        const auto rot     = glm::quat(body.rotation());
        const glm::vec3 lv = glm::vec3(body.getLinearVelocity());
        const glm::vec3 av = glm::vec3(body.getAngularVelocity());
        const auto warp    = glm::vec3(ship.getWarpSpeedVector3());

        wx                 = warp.x;
        wy                 = warp.y;
        wz                 = warp.z;

        data               = to_string(anchorList.size()); //[0]
        for (auto& closest : anchorList)
            data += "," + closest;
        const auto nearestAnchorPos = finalAnchor->getPosition();

        px = static_cast<float>(pos.x - nearestAnchorPos.x);
        py = static_cast<float>(pos.y - nearestAnchorPos.y);
        pz = static_cast<float>(pos.z - nearestAnchorPos.z);

        qXYZ = Math::pack3NormalsInto32Int(rot.x, rot.y, rot.z);
        Math::Float16From32(&qw, rot.w);

        Math::Float16From32(&lx, lv.x);
        Math::Float16From32(&ly, lv.y);
        Math::Float16From32(&lz, lv.z);

        Math::Float16From32(&ax, av.x);
        Math::Float16From32(&ay, av.y);
        Math::Float16From32(&az, av.z);
    }
}
bool PacketPhysicsUpdate::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> px >> py >> pz >> wx >> wy >> wz >> qXYZ >> qw >> lx >> ly >> lz >> ax >> ay >> az >> team_number >> ai_type >> ship_class >> ship_map_key >> player_username >> data);
}
bool PacketPhysicsUpdate::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << px << py << pz << wx << wy << wz << qXYZ << qw << lx << ly << lz << ax << ay << az << team_number << ai_type << ship_class << ship_map_key << player_username << data);
}
//void PacketPhysicsUpdate::print() {}
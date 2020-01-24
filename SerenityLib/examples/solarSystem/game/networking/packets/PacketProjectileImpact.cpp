#include "PacketProjectileImpact.h"

using namespace std;

PacketProjectileImpact::PacketProjectileImpact() {
    damage = impactX = impactY = impactZ = 0.0f;
    impacted_ship_map_key = source_ship_map_key = "";
    time = radius = normalZ = normalY = normalX = projectile_index = model_index = shields = shield_side = 0;
}
bool PacketProjectileImpact::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> impactX >> impactY >> impactZ >> damage >> normalX >> normalY >> normalZ >> time >> radius >> shields >> projectile_index >> model_index >> shield_side >> source_ship_map_key >> impacted_ship_map_key);
}
bool PacketProjectileImpact::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << impactX << impactY << impactZ << damage << normalX << normalY << normalZ << time << radius << shields << projectile_index << model_index << shield_side << source_ship_map_key << impacted_ship_map_key);
}
//void PacketProjectileImpact::print() {}
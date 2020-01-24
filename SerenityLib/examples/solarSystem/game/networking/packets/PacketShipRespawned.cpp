#include "PacketShipRespawned.h"

using namespace std;


PacketShipRespawnNotification::PacketShipRespawnNotification() {
    timer = 0;
}
bool PacketShipRespawnNotification::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> timer);
}
bool PacketShipRespawnNotification::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << timer);
}


PacketShipRespawned::PacketShipRespawned() {
    x = y = z = 0.0f;
    respawned_ship_map_key = nearest_spawn_anchor_name = "";
}
bool PacketShipRespawned::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> x >> y >> z >> respawned_ship_map_key >> nearest_spawn_anchor_name);
}
bool PacketShipRespawned::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << x << y << z << respawned_ship_map_key << nearest_spawn_anchor_name);
}
//void PacketShipRespawned::print(){}
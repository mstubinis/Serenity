#include "PacketMapData.h"

using namespace std;

PacketMapData::PacketMapData() {
    map_name = "";
    map_file_name = "";
    map_allowed_ships = "";
}
bool PacketMapData::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> map_name >> map_file_name >> map_allowed_ships);
}
bool PacketMapData::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << map_name << map_file_name << map_allowed_ships);
}
//void PacketMessage::print() {}
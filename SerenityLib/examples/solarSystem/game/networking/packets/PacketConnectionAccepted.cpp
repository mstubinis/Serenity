#include "PacketConnectionAccepted.h"

#include "../../modes/GameplayMode.h"

using namespace std;

PacketConnectionAccepted::PacketConnectionAccepted() {
    already_connected_players = allowed_ships = map_name = map_file_name = "";
    game_mode_type = static_cast<unsigned char>(GameplayModeType::FFA);
    is_host = false;
}
bool PacketConnectionAccepted::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> already_connected_players >> allowed_ships >> map_name >> map_file_name >> game_mode_type >> is_host);
}
bool PacketConnectionAccepted::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << already_connected_players << allowed_ships << map_name << map_file_name << game_mode_type << is_host);
}
//void PacketConnectionAccepted::print() {}
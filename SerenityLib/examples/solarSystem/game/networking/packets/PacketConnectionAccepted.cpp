#include "PacketConnectionAccepted.h"

#include "../../modes/GameplayMode.h"

using namespace std;

PacketConnectionAccepted::PacketConnectionAccepted() {
    already_connected_players = allowed_ships = map_name = map_file_name = "";
    game_mode_type = static_cast<unsigned char>(GameplayModeType::FFA);
    is_host = false;
    lobby_time_left = 0.0;
}
bool PacketConnectionAccepted::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> already_connected_players >> allowed_ships >> map_name >> map_file_name >> game_mode_type >> is_host >> lobby_time_left);
}
bool PacketConnectionAccepted::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << already_connected_players << allowed_ships << map_name << map_file_name << game_mode_type << is_host << lobby_time_left);
}
//void PacketConnectionAccepted::print() {}
#include "PacketGameplayModeInfo.h"

using namespace std;


PacketGameplayModeInfo::PacketGameplayModeInfo() {
    gameplay_mode_type = gameplay_mode_max_number_of_players = gameplay_mode_team_sizes = 0;
    allowed_ships = "";
    team_data = "";
}
bool PacketGameplayModeInfo::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> gameplay_mode_type >> gameplay_mode_max_number_of_players >> gameplay_mode_team_sizes >> allowed_ships >> team_data);
}
bool PacketGameplayModeInfo::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << gameplay_mode_type << gameplay_mode_max_number_of_players << gameplay_mode_team_sizes << allowed_ships << team_data);
}
//void PacketGameplayModeInfo::print(){}

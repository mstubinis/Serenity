#include "PacketGameplayModeInfo.h"

using namespace std;


PacketGameplayModeInfo::PacketGameplayModeInfo() {
    gameplay_mode_type = gameplay_mode_max_number_of_players = gameplay_mode_team_sizes = 0;
    data = "";
}
bool PacketGameplayModeInfo::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> data >> gameplay_mode_type >> gameplay_mode_max_number_of_players >> gameplay_mode_team_sizes);
}
bool PacketGameplayModeInfo::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << data << gameplay_mode_type << gameplay_mode_max_number_of_players << gameplay_mode_team_sizes);
}
//void PacketGameplayModeInfo::print(){}

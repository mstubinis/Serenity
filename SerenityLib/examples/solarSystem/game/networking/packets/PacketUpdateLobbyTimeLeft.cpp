#include "PacketUpdateLobbyTimeLeft.h"

using namespace std;

PacketUpdateLobbyTimeLeft::PacketUpdateLobbyTimeLeft() {
    time_left = 0.0;
}
bool PacketUpdateLobbyTimeLeft::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> time_left);
}
bool PacketUpdateLobbyTimeLeft::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << time_left);
}
//void PacketUpdateLobbyTimeLeft::print() {}
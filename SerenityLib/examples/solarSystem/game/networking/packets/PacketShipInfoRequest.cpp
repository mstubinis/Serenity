#include "PacketShipInfoRequest.h"

using namespace std;

PacketShipInfoRequest::PacketShipInfoRequest() {
    ship_that_wants_info_key = requested_ship_key = requested_ship_username = "";
}
bool PacketShipInfoRequest::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> ship_that_wants_info_key >> requested_ship_key >> requested_ship_username);
}
bool PacketShipInfoRequest::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << ship_that_wants_info_key << requested_ship_key << requested_ship_username);
}
//void PacketShipInfoRequest::print(){}
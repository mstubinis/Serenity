#include "PacketAuthStep1.h"

using namespace std;

PacketAuthStep1::PacketAuthStep1() {
    secret_key = sf::Uint32(0);
    server_nonce = sf::Uint32(0);
}
bool PacketAuthStep1::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> server_nonce >> secret_key);
}
bool PacketAuthStep1::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << server_nonce << secret_key);
}
//void PacketMessage::print() {}
#include "PacketAuthStep2.h"

using namespace std;

PacketAuthStep2::PacketAuthStep2() {
    server_nonce    = sf::Uint32(0);
    client_nonce    = sf::Uint32(0);
    username        = "";
    password        = "";
}
bool PacketAuthStep2::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> server_nonce >> client_nonce >> username >> password);
}
bool PacketAuthStep2::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << server_nonce << client_nonce << username << password);
}
//void PacketMessage::print() {}
#include "PacketMessage.h"

using namespace std;

PacketMessage::PacketMessage() {
    data = "";
    name = "";
    r = g = b = 1.0f;
}
bool PacketMessage::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> data >> name >> r >> g >> b);
}
bool PacketMessage::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << data << name << r << g << b);
}
//void PacketMessage::print() {}
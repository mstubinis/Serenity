#include "PacketCollisionEvent.h"

using namespace std;

PacketCollisionEvent::PacketCollisionEvent() {
    damage1 = damage2 = 0.0;
    owner_key = other_key = "";
    lx1 = ly1 = lz1 = ax1 = ay1 = az1 = lx2 = ly2 = lz2 = ax2 = ay2 = az2 = 0;
}
bool PacketCollisionEvent::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> damage1 >> damage2 >> lx1 >> ly1 >> lz1 >> ax1 >> ay1 >> az1 >> lx2 >> ly2 >> lz2 >> ax2 >> ay2 >> az2 >> owner_key >> other_key);
}
bool PacketCollisionEvent::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << damage1 << damage2 << lx1 << ly1 << lz1 << ax1 << ay1 << az1 << lx2 << ly2 << lz2 << ax2 << ay2 << az2 << owner_key << other_key);
}
//void PacketCollisionEvent::print() {}
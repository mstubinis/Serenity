#include "PacketClientRequestConnectionToServer.h"

using namespace std;

PacketClientRequestConnectionToServer::PacketClientRequestConnectionToServer() {
    shipName = "";
}
bool PacketClientRequestConnectionToServer::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> shipName >> mshIndex >> mshVersion >> mshType >> matIndex >> matVersion >> matType);
}
bool PacketClientRequestConnectionToServer::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << shipName << mshIndex << mshVersion << mshType << matIndex << matVersion << matType);
}
//void PacketClientRequestConnectionToServer::print() {}
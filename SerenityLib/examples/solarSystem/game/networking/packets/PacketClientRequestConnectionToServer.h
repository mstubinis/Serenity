#pragma once
#ifndef GAME_PACKET_CLIENT_REQUEST_CONNECTION_TO_SERVER_H
#define GAME_PACKET_CLIENT_REQUEST_CONNECTION_TO_SERVER_H

#include "Packet.h"

struct PacketClientRequestConnectionToServer : public Packet {
    std::string   shipName;

    std::uint32_t mshIndex;
    std::uint32_t mshVersion;
    std::uint32_t mshType;

    std::uint32_t matIndex;
    std::uint32_t matVersion;
    std::uint32_t matType;

    PacketClientRequestConnectionToServer();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
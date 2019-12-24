#pragma once
#ifndef GAME_PACKET_MAP_DATA_H
#define GAME_PACKET_MAP_DATA_H

#include "Packet.h"

struct PacketMapData : public Packet {
    std::string   map_name;
    std::string   map_file_name;
    std::string   map_allowed_ships;

    PacketMapData();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
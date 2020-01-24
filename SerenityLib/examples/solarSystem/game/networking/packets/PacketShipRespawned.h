#pragma once
#ifndef GAME_PACKET_SHIP_RESPAWNED_H
#define GAME_PACKET_SHIP_RESPAWNED_H

#include "Packet.h"


struct PacketShipRespawnNotification : public Packet {
    float         timer;

    PacketShipRespawnNotification();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

struct PacketShipRespawned : public Packet {
    float         x, y, z;
    std::string   respawned_ship_map_key;
    std::string   nearest_spawn_anchor_name;

    PacketShipRespawned();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
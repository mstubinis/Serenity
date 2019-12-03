#pragma once
#ifndef GAME_PACKETS_PROJECTILE_IMPACT_H
#define GAME_PACKETS_PROJECTILE_IMPACT_H

#include "Packet.h"

struct PacketProjectileImpact : public Packet {
    float    impactX, 
             impactY, 
             impactZ, 
             damage;

    uint16_t normalX, 
             normalY, 
             normalZ, 
             time, 
             radius;

    bool     shields;

    int      projectile_index,
             model_index,
             shield_side;

    std::string source_ship_map_key;
    std::string impacted_ship_map_key;

    PacketProjectileImpact();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
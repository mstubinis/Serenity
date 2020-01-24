#pragma once
#ifndef GAME_PACKET_COLLISION_EVENT_H
#define GAME_PACKET_COLLISION_EVENT_H

#include "Packet.h"

struct PacketCollisionEvent : public Packet {
    float            damage1, damage2;
    uint16_t         lx1, ly1, lz1;     //linear velocity
    uint16_t         ax1, ay1, az1;     //angular velocity

    uint16_t         lx2, ly2, lz2;     //linear velocity
    uint16_t         ax2, ay2, az2;     //angular velocity

    std::string      owner_key;
    std::string      other_key;

    PacketCollisionEvent();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
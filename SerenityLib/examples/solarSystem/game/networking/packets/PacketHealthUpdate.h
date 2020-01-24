#pragma once
#ifndef GAME_PACKETS_HEALTH_UPDATE_H
#define GAME_PACKETS_HEALTH_UPDATE_H

#include "Packet.h"

class  Ship;
struct PacketHealthUpdate : public Packet {
    struct PacketHealthFlags final { enum Flag {
        None = 0,
        ShieldsInstalled = 1 << 0,
        ShieldsActive = 1 << 1,
        ShieldsTurnedOn = 1 << 2,
        All = 4294967295,
    };};
    float        currentHullHealth,
                 currentShieldsHealthF,
                 currentShieldsHealthA,
                 currentShieldsHealthP,
                 currentShieldsHealthS,
                 currentShieldsHealthD,
                 currentShieldsHealthV;

    unsigned int flags;

    std::string ship_class;
    std::string ship_map_key;

    PacketHealthUpdate();
    PacketHealthUpdate(Ship& ship);
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
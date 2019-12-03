#pragma once
#ifndef GAME_PACKETS_CLOAK_UPDATE_H
#define GAME_PACKETS_CLOAK_UPDATE_H

#include "Packet.h"

class  Ship;
struct PacketCloakUpdate : public Packet {

    uint16_t cloakTimer;

    bool cloakSystemOnline, 
         cloakActive, 
         justTurnedOn, 
         justTurnedOff;

    std::string ship_map_key;

    PacketCloakUpdate();
    PacketCloakUpdate(Ship& ship);
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
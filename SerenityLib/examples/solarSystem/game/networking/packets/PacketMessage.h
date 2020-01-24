#pragma once
#ifndef GAME_PACKET_MESSAGE_H
#define GAME_PACKET_MESSAGE_H

#include "Packet.h"

struct PacketMessage : public Packet {
    std::string   name;
    std::string   data;
    float         r, g, b;

    PacketMessage();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
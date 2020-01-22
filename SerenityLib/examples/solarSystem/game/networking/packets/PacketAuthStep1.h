#pragma once
#ifndef GAME_PACKET_AUTH_STEP_1_H
#define GAME_PACKET_AUTH_STEP_1_H

#include "Packet.h"

struct PacketAuthStep1 : public Packet {
    sf::Uint32 secret_key;
    sf::Uint32 server_nonce;

    PacketAuthStep1();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
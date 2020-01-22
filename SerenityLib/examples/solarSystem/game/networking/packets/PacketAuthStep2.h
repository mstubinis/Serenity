#pragma once
#ifndef GAME_PACKET_AUTH_STEP_2_H
#define GAME_PACKET_AUTH_STEP_2_H

#include "Packet.h"


struct PacketAuthStep2 : public Packet {

    sf::Uint32 server_nonce;
    sf::Uint32 client_nonce;

    std::string username; //should be encrypted
    std::string password; //should be hashed as hash(client_nonce + server_nonce + password_from_db)

    PacketAuthStep2();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
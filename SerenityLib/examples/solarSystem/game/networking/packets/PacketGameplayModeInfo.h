#pragma once
#ifndef GAME_PACKET_GAMEPLAY_MODE_INFO_H
#define GAME_PACKET_GAMEPLAY_MODE_INFO_H

#include "Packet.h"

struct PacketGameplayModeInfo : public Packet {

    unsigned int  gameplay_mode_type,
                  gameplay_mode_max_number_of_players,
                  gameplay_mode_team_sizes;


    std::string   allowed_ships;
    std::string   team_data;


    PacketGameplayModeInfo();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
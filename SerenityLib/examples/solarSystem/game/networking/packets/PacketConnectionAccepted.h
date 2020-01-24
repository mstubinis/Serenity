#pragma once
#ifndef GAME_PACKET_CONNECTION_ACCEPTED_H
#define GAME_PACKET_CONNECTION_ACCEPTED_H

#include "Packet.h"

struct PacketConnectionAccepted : public Packet {
    std::string    already_connected_players;
    std::string    allowed_ships;

    std::string    map_name;
    std::string    map_file_name;

    unsigned char  game_mode_type;
    bool           is_host;

    double         lobby_time_left;

    PacketConnectionAccepted();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
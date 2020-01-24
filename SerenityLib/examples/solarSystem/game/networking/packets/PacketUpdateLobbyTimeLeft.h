#pragma once
#ifndef GAME_PACKET_UPDATE_LOBBY_TIME_LEFT_H
#define GAME_PACKET_UPDATE_LOBBY_TIME_LEFT_H

#include "Packet.h"

struct PacketUpdateLobbyTimeLeft : public Packet {
    double time_left;

    PacketUpdateLobbyTimeLeft();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
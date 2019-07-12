#pragma once
#ifndef GAME_PACKET_H
#define GAME_PACKET_H


#include <string>
#include <iostream>

struct PacketType {enum Type {
    ServerShutdown,
    ClientDisconnect,
    ClientReconnect,
    ClientConnectionAccepted,
    ClientConnectionRejected,

    ClientSendInfo,
};};


struct Packet
{
    unsigned char    PacketType; //0 - 255, up this if needed (when/if we need more types)

    //all of these are half floats for now to save on packet size
    uint16_t         x;
    uint16_t         y;
    uint16_t         z;

    uint16_t         x1;
    uint16_t         y1;
    uint16_t         z1;

    uint16_t         x2;
    uint16_t         y2;
    uint16_t         z2;

    uint16_t         x3;
    uint16_t         y3;
    uint16_t         z3;

    void print() {

    }
};



#endif
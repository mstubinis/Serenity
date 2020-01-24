#pragma once
#ifndef GAME_PACKET_PHYSICS_UPDATE_H
#define GAME_PACKET_PHYSICS_UPDATE_H

#include "Packet.h"

class  Ship;
class  Map;
class  Anchor;
struct PacketPhysicsUpdate : public Packet {

    float            px, py, pz;     //position
    float            wx, wy, wz;     //warp speed
    unsigned int     qXYZ;           //rotation normals as 32 bit unsigned int
    uint16_t         qw;             //rotation W component
    uint16_t         lx, ly, lz;     //linear velocity
    uint16_t         ax, ay, az;     //angular velocity


    unsigned int     team_number,
                     ai_type;

    std::string      ship_class;
    std::string      ship_map_key;
    std::string      player_username;
    std::string      data;

    PacketPhysicsUpdate();
    PacketPhysicsUpdate(Ship& ship, Map& map, Anchor* closestAnchor, const std::vector<std::string>& anchorList, const std::string& username);
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};

#endif
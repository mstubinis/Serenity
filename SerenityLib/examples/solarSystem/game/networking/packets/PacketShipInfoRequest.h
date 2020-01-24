#pragma once
#ifndef GAME_PACKET_SHIP_INFO_REQUEST_H
#define GAME_PACKET_SHIP_INFO_REQUEST_H


#include "Packet.h"


/*
                PacketMessage pOut;
                pOut.PacketType = PacketType::Client_To_Server_Request_Ship_Current_Info;
                pOut.name       = player_you->getMapKey();
                pOut.data       = new_ship->getMapKey();
                pOut.data      += "," + pI.player_username;
                send(pOut);
*/

struct PacketShipInfoRequest : public Packet {
    std::string   ship_that_wants_info_key,
                  requested_ship_key,
                  requested_ship_username;

    PacketShipInfoRequest();
    bool validate(sf::Packet& sfPacket);
    bool build(sf::Packet& sfPacket);
    //void print();
};


#endif
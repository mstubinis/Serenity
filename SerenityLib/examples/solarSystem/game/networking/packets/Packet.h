#pragma once
#ifndef GAME_PACKET_H
#define GAME_PACKET_H

#include <string>
#include <iostream>

#include <SFML/Network/Packet.hpp>

struct PacketType {enum Type: unsigned int {
    Undefined,
    Server_Shutdown,

    Server_To_Client_Notify_Ship_Of_Respawn,

    Server_To_Client_Notify_Ship_Of_Impending_Respawn,

    Client_To_Server_Ship_Was_Just_Destroyed,
    Server_To_Client_Ship_Was_Just_Destroyed,

    Client_To_Server_Request_Ship_Current_Info,
    Server_To_Client_Request_Ship_Current_Info,

    Client_To_Server_Collision_Event,
    Server_To_Client_Collision_Event,

    Client_To_Server_Anti_Cloak_Status,
    Server_To_Client_Anti_Cloak_Status,


    Client_To_Server_Request_GameplayMode,
    Server_To_Client_Request_GameplayMode,

    Client_To_Server_Projectile_Cannon_Impact,
    Client_To_Server_Projectile_Torpedo_Impact,
    Server_To_Client_Projectile_Cannon_Impact,
    Server_To_Client_Projectile_Torpedo_Impact,


    Client_To_Server_Ship_Health_Update,
    Server_To_Client_Ship_Health_Update,

    Server_To_Client_Client_Left_Map,

    Client_To_Server_Client_Fired_Cannons,
    Client_To_Server_Client_Fired_Beams,
    Client_To_Server_Client_Fired_Torpedos,

    Server_To_Client_Client_Fired_Cannons,
    Server_To_Client_Client_Fired_Beams,
    Server_To_Client_Client_Fired_Torpedos,


    Client_To_Server_Client_Changed_Target,
    Server_To_Client_Client_Changed_Target,

    Client_To_Server_Request_Connection,
    Client_To_Server_Request_Disconnection,
    Client_To_Server_Ship_Physics_Update,
    Client_To_Server_Ship_Cloak_Update,

    Client_To_Server_Chat_Message,

    Client_To_Server_Request_Anchor_Creation,
    Client_To_Server_Request_Map_Entry,
    Client_To_Server_Successfully_Entered_Map,

    Server_To_Client_Approve_Map_Entry,
    Server_To_Client_Reject_Map_Entry,

    Server_To_Client_Anchor_Creation,
    Server_To_Client_Anchor_Creation_Deep_Space_Initial,
    Server_To_Client_New_Client_Entered_Map,

    Server_To_Client_Send_Basic_Server_Info,
    Server_To_Client_Ship_Physics_Update,
    Server_To_Client_Ship_Cloak_Update,
    Server_To_Client_Accept_Connection,
    Server_To_Client_Reject_Connection,

    Server_To_Client_Chat_Message,
    Server_To_Client_Map_Data,

    Server_To_Client_Client_Joined_Server,
    Server_To_Client_Client_Left_Server,
};};

struct IPacket {
    virtual bool validate(sf::Packet& sfPacket) = 0;
    virtual bool build(sf::Packet& sfPacket) = 0;
    virtual void print() = 0;
};

struct Packet: public IPacket {
    unsigned int    PacketType;
    Packet() {
        PacketType  = 0;
    }
    virtual bool validate(sf::Packet& sfPacket){
        return (sfPacket >> PacketType);
    }
    virtual bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType);
    }
    virtual void print() {
    }
    static Packet* getPacket(const sf::Packet& sfPacket);
};

#endif
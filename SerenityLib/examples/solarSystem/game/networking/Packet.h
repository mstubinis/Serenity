#pragma once
#ifndef GAME_PACKET_H
#define GAME_PACKET_H


#include <string>
#include <iostream>

#include <SFML/Network/Packet.hpp>
#include <glm/glm.hpp>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <core/engine/resources/Handle.h>

class  Ship;
class  Map;
class  Anchor;
struct PacketType {enum Type: unsigned int {
    Undefined,
    Server_Shutdown,

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

    Client_To_Server_Periodic_Ping,

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
    unsigned int   PacketType;
    std::string     data;
    Packet() {
        PacketType = static_cast<unsigned int>(PacketType::Undefined);
        data = "";
    }
    virtual bool validate(sf::Packet& sfPacket){
        return (sfPacket >> PacketType >> data);
    }
    virtual bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data);
    }
    virtual void print() {}
    static Packet* getPacket(const sf::Packet& sfPacket);
};

struct PacketProjectileImpact : public Packet {
    float impactX, impactY, impactZ, damage;
    uint16_t normalX, normalY, normalZ, time, radius; //half floats
    bool shields;
    int index;
    PacketProjectileImpact() {
        damage = 0.0f;
        time = radius = normalZ = normalY = normalX = index = shields = 0;
    }
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> impactX >> impactY >> impactZ >> damage >> normalX >> normalY >> normalZ >> time >> radius >> shields >> index);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << impactX << impactY << impactZ << damage << normalX << normalY << normalZ << time << radius << shields << index);
    }
    void print() {}
};
struct PacketHealthUpdate : public Packet {
    struct PacketHealthFlags final { enum Flag {
        None              = 0,
        ShieldsInstalled  = 1 << 0,
        ShieldsActive     = 1 << 1,
        ShieldsTurnedOn   = 1 << 2,
        All               = 4294967295,
    };};
    float        currentHullHealth;

    float        currentShieldsHealthF;
    float        currentShieldsHealthA;
    float        currentShieldsHealthP;
    float        currentShieldsHealthS;
    float        currentShieldsHealthD;
    float        currentShieldsHealthV;

    unsigned int flags;

    PacketHealthUpdate();
    PacketHealthUpdate(Ship& ship);
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> currentHullHealth >> currentShieldsHealthF >> currentShieldsHealthA >> currentShieldsHealthP >> currentShieldsHealthS >> currentShieldsHealthD >> currentShieldsHealthV >> flags);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << currentHullHealth << currentShieldsHealthF << currentShieldsHealthA << currentShieldsHealthP << currentShieldsHealthS << currentShieldsHealthD << currentShieldsHealthV << flags);
    }
    void print() {}
};
struct PacketPhysicsUpdate: public Packet {
    //256 bits (64 bytes)
    float            px, py, pz;     //position
    float            wx, wy, wz;     //warp speed
    unsigned int     qXYZ;
    uint16_t         qw;
    uint16_t         lx, ly, lz;     //linear velocity
    uint16_t         ax, ay, az;     //angular velocity
    PacketPhysicsUpdate();
    PacketPhysicsUpdate(Ship& ship, Map& map, Anchor* closestAnchor, const std::vector<std::string>& anchorList, const std::string& username);
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> px >> py >> pz >> wx >> wy >> wz >> qXYZ >> qw >> lx >> ly >> lz >> ax >> ay >> az);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << px << py << pz << wx << wy << wz << qXYZ << qw << lx << ly << lz << ax << ay << az);
    }
    void print() {}
};
struct PacketCloakUpdate : public Packet {
    float cloakTimer;
    bool cloakSystemOnline;
    bool cloakActive;
    bool justTurnedOn;
    bool justTurnedOff;
    PacketCloakUpdate();
    PacketCloakUpdate(Ship& ship);
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> cloakTimer >> cloakSystemOnline >> cloakActive >> justTurnedOn >> justTurnedOff);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << cloakTimer << cloakSystemOnline << cloakActive << justTurnedOn << justTurnedOff);
    }
    void print() {}
};
struct PacketClientRequestConnectionToServer : public Packet {
    std::string  shipName;

    std::uint32_t mshIndex;
    std::uint32_t mshVersion;
    std::uint32_t mshType;

    std::uint32_t matIndex;
    std::uint32_t matVersion;
    std::uint32_t matType;

    PacketClientRequestConnectionToServer() {
        shipName = "";
    }
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> shipName >> mshIndex >> mshVersion >> mshType >> matIndex >> matVersion >> matType);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << shipName << mshIndex << mshVersion << mshType << matIndex << matVersion << matType);
    }
    void print() {}
};


struct PacketMessage : public Packet {
    std::string   name;
    float         r, g, b;
    PacketMessage() {
        name = "";
        r = g = b = 1.0f;
    }
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> name >> r >> g >> b);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << name << r << g << b);
    }
    void print() {}
};


#endif
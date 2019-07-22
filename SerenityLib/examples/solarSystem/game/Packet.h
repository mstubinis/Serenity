#pragma once
#ifndef GAME_PACKET_H
#define GAME_PACKET_H


#include <string>
#include <iostream>

#include <SFML/Network/Packet.hpp>
#include <glm/glm.hpp>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <core/engine/resources/Handle.h>

class Ship;

struct PacketType {enum Type {
    Undefined,
    Server_Shutdown,

    Client_To_Server_Request_Connection,
    Client_To_Server_Request_Disconnection,
    Client_To_Server_Ship_Physics_Update,

    Client_To_Server_Chat_Message,

    Server_To_Client_Send_Basic_Server_Info,
    Server_To_Client_Ship_Physics_Update,
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
    unsigned char    PacketType; //0 - 255, up this if needed (when/if we need more types)
    std::string      data;
    Packet() {
        PacketType = static_cast<unsigned char>(PacketType::Undefined);
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

struct PacketPhysicsUpdate: public Packet {
    //256 bits (64 bytes)
    float            px, py, pz;     //position
    uint16_t         qx, qy, qz, qw; //rotation
    uint16_t         lx, ly, lz;     //linear velocity
    uint16_t         ax, ay, az;     //angular velocity
    PacketPhysicsUpdate();
    PacketPhysicsUpdate(Ship& ship);
    bool validate(sf::Packet& sfPacket) {
        return (sfPacket >> PacketType >> data >> px >> py >> pz >> qx >> qy >> qz >> qw >> lx >> ly >> lz >> ax >> ay >> az);
    }
    bool build(sf::Packet& sfPacket) {
        return (sfPacket << PacketType << data << px << py << pz << qx << qy << qz << qw << lx << ly << lz << ax << ay << az);
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


struct PacketChatMessage : public Packet {
    std::string  name;
    float r;
    float g;
    float b;

    PacketChatMessage() {
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
#include "Packet.h"
#include "Ship.h"
#include "map/Anchor.h"
#include "map/Map.h"

#include <core/engine/math/Engine_Math.h>
#include <ecs/ComponentBody.h>

#include "ships/shipSystems/ShipSystemCloakingDevice.h"

using namespace Engine;
using namespace std;

Packet* Packet::getPacket(const sf::Packet& sfPacket) {
    char* data = (char*)(sfPacket.getData());
    unsigned int packetType = static_cast<unsigned int>(data[0]);
    Packet* p = nullptr;

    switch (packetType) {
        case PacketType::Server_To_Client_Accept_Connection: {
            p = new Packet(); break;
        }case PacketType::Server_To_Client_Reject_Connection: {
            p = new Packet(); break;
        }case PacketType::Client_To_Server_Request_Connection: {
            p = new Packet(); break;
        }case PacketType::Client_To_Server_Request_Disconnection: {
            p = new Packet(); break;
        }case PacketType::Server_Shutdown: {
            p = new Packet(); break;
        }case PacketType::Client_To_Server_Ship_Physics_Update: {
            p = new PacketPhysicsUpdate(); break;
        }case PacketType::Server_To_Client_Ship_Physics_Update: {
            p = new PacketPhysicsUpdate(); break;
        }case PacketType::Client_To_Server_Chat_Message: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Chat_Message: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Client_Joined_Server: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Client_Left_Server: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Map_Data: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Request_Map_Entry: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Approve_Map_Entry: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Reject_Map_Entry: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_New_Client_Entered_Map: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Request_Anchor_Creation:{
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Anchor_Creation: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Successfully_Entered_Map: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Ship_Cloak_Update:{
            p = new PacketCloakUpdate(); break;
        }case PacketType::Server_To_Client_Ship_Cloak_Update: {
            p = new PacketCloakUpdate(); break;
        }default: {
            break;
        }
    }
    if(!p)
        cout << "Invalid packet type in getPacket(), please see Packet.cpp" << endl;
    return p;
}


PacketPhysicsUpdate::PacketPhysicsUpdate():Packet() {
    qx = qy = qz = lx = ly = lz = ax = ay = az = 0;
    px = py = pz = wx = wy = wz = 0.0f;
    qw = 1;
}
PacketPhysicsUpdate::PacketPhysicsUpdate(Ship& ship, Map& map, Anchor* finalAnchor, const vector<string>& anchorList) :Packet() {
    auto& ent = ship.entity();
    EntityDataRequest request(ent);
    const auto pbody = ent.getComponent<ComponentBody>(request);
    const auto pname = ent.getComponent<ComponentName>(request);

    data += ship.getClass();
    if (pname)
        data += ("," + pname->name());
    if (pbody) {
        auto& body = *pbody;

        const auto& pos = body.position();
        const auto& rot = body.rotation();
        const auto& lv  = body.getLinearVelocity();
        const auto& av  = body.getAngularVelocity();
        const auto warp = ship.getWarpSpeedVector3();

        wx = warp.x;
        wy = warp.y;
        wz = warp.z;
        
        data += "," + to_string(anchorList.size());
        for (auto& closest : anchorList)
            data += "," + closest;
        const auto nearestAnchorPos = finalAnchor->getPosition();

        px = pos.x - nearestAnchorPos.x;
        py = pos.y - nearestAnchorPos.y;
        pz = pos.z - nearestAnchorPos.z;

        Math::Float16From32(&qx, rot.x);
        Math::Float16From32(&qy, rot.y);
        Math::Float16From32(&qz, rot.z);
        Math::Float16From32(&qw, rot.w);

        Math::Float16From32(&lx, lv.x);
        Math::Float16From32(&ly, lv.y);
        Math::Float16From32(&lz, lv.z);

        Math::Float16From32(&ax, av.x);
        Math::Float16From32(&ay, av.y);
        Math::Float16From32(&az, av.z);
    }
}



PacketCloakUpdate::PacketCloakUpdate() :Packet() {
    cloakTimer = 1.0f;
    cloakSystemOnline = false;
    cloakActive = false;
    justTurnedOn = false;
    justTurnedOff = false;
}
PacketCloakUpdate::PacketCloakUpdate(Ship& ship) : Packet() {
    auto& ent = ship.entity();
    EntityDataRequest request(ent);
    const auto pname = ent.getComponent<ComponentName>(request);
    justTurnedOn = false;
    justTurnedOff = false;
    data += ship.getClass();
    if (pname)
        data += ("," + pname->name());
    if (ship.getShipSystem(ShipSystemType::CloakingDevice)) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(ship.getShipSystem(ShipSystemType::CloakingDevice));
        cloakTimer = cloak.getCloakTimer();
        cloakSystemOnline = cloak.isOnline();
        cloakActive = cloak.isCloakActive();
    }
}

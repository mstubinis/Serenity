#include "Packet.h"
#include "../Ship.h"
#include "../map/Anchor.h"
#include "../map/Map.h"

#include <core/engine/math/Engine_Math.h>
#include <ecs/ComponentBody.h>

#include "../ships/shipSystems/ShipSystemCloakingDevice.h"
#include "../ships/shipSystems/ShipSystemMainThrusters.h"
#include "../ships/shipSystems/ShipSystemPitchThrusters.h"
#include "../ships/shipSystems/ShipSystemReactor.h"
#include "../ships/shipSystems/ShipSystemRollThrusters.h"
#include "../ships/shipSystems/ShipSystemSensors.h"
#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemWarpDrive.h"
#include "../ships/shipSystems/ShipSystemYawThrusters.h"
#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "../ships/shipSystems/ShipSystemHull.h"

using namespace Engine;
using namespace std;

Packet* Packet::getPacket(const sf::Packet& sfPacket) {
    sf::Packet hardCopy = sf::Packet(sfPacket);
    Packet pp;
    pp.validate(hardCopy);
    const unsigned int packetType = pp.PacketType;
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
        }case PacketType::Client_To_Server_Client_Changed_Target: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Client_Changed_Target: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Client_Fired_Cannons:{
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Client_Fired_Beams:{
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Client_Fired_Torpedos: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Client_Fired_Cannons: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Client_Fired_Beams: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Client_Fired_Torpedos: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Periodic_Ping: {
            p = new Packet(); break;
        }case PacketType::Server_To_Client_Client_Left_Map: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Ship_Health_Update: {
            p = new PacketHealthUpdate(); break;
        }case PacketType::Server_To_Client_Ship_Health_Update: {
            p = new PacketHealthUpdate(); break;
        }case PacketType::Client_To_Server_Projectile_Cannon_Impact: {
            p = new PacketProjectileImpact(); break;
        }case PacketType::Client_To_Server_Projectile_Torpedo_Impact: {
            p = new PacketProjectileImpact(); break;
        }case PacketType::Server_To_Client_Projectile_Cannon_Impact: {
            p = new PacketProjectileImpact(); break;
        }case PacketType::Server_To_Client_Projectile_Torpedo_Impact: {
            p = new PacketProjectileImpact(); break;
        }case PacketType::Client_To_Server_Request_GameplayMode: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Request_GameplayMode: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Anti_Cloak_Status: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Anti_Cloak_Status: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Collision_Event: {
            p = new PacketCollisionEvent(); break;
        }case PacketType::Server_To_Client_Collision_Event: {
            p = new PacketCollisionEvent(); break;
        }case PacketType::Client_To_Server_Request_Ship_Current_Info: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Request_Ship_Current_Info: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Ship_Was_Just_Destroyed: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Ship_Was_Just_Destroyed: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Notify_Ship_Of_Impending_Respawn: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Notify_Ship_Of_Respawn: {
            p = new PacketMessage(); break;
        }default: {
            break;
        }
    }
    if(!p)
        cout << "Invalid packet type in getPacket(), please see Packet.cpp" << endl;
    return p;
}
PacketHealthUpdate::PacketHealthUpdate() : Packet() {
    currentHullHealth     = 0;
    currentShieldsHealthF = 0;
    currentShieldsHealthA = 0;
    currentShieldsHealthP = 0;
    currentShieldsHealthS = 0;
    currentShieldsHealthD = 0;
    currentShieldsHealthV = 0;
    flags                 = PacketHealthFlags::None;
}
PacketHealthUpdate::PacketHealthUpdate(Ship& ship) : PacketHealthUpdate() {
    auto* shields = static_cast<ShipSystemShields*>(ship.getShipSystem(ShipSystemType::Shields));
    auto* hull = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));
    if (shields) {
        auto& shield = *shields;

        currentShieldsHealthF = shield.getActualShieldHealthCurrent(0);
        currentShieldsHealthA = shield.getActualShieldHealthCurrent(1);
        currentShieldsHealthP = shield.getActualShieldHealthCurrent(2);
        currentShieldsHealthS = shield.getActualShieldHealthCurrent(3);
        currentShieldsHealthD = shield.getActualShieldHealthCurrent(4);
        currentShieldsHealthV = shield.getActualShieldHealthCurrent(5);

        flags = flags | PacketHealthFlags::ShieldsInstalled;
        if (shield.shieldsAreUp()) {
            flags = flags | PacketHealthFlags::ShieldsTurnedOn;
        }
        if (shield.isOnline()) {
            flags = flags | PacketHealthFlags::ShieldsActive;
        }
    }
    if (hull) {
        currentHullHealth = hull->getHealthCurrent();
    }
    data += ship.getClass();
    data += "," + ship.getMapKey();
}

PacketPhysicsUpdate::PacketPhysicsUpdate() : Packet() {
    qXYZ = lx = ly = lz = ax = ay = az = 0;
    px = py = pz = wx = wy = wz = 0.0f;
    qw = 1;
}
PacketPhysicsUpdate::PacketPhysicsUpdate(Ship& ship, Map& map, Anchor* finalAnchor, const vector<string>& anchorList, const string& username) : PacketPhysicsUpdate() {
    auto& ent = ship.entity();
    EntityDataRequest request(ent);
    const auto pbody = ent.getComponent<ComponentBody>(request);

    data += ship.getClass(); //[0]
    data += "," + ship.getMapKey(); //[1]
    data += "," + username; //[2]
    if (pbody) {
        auto& body = *pbody;

        const auto pos      = body.position();
        const auto rot      = glm::quat(body.rotation());
        const glm::vec3 lv  = glm::vec3(body.getLinearVelocity());
        const glm::vec3 av  = glm::vec3(body.getAngularVelocity());
        const auto warp     = glm::vec3(ship.getWarpSpeedVector3());

        wx = warp.x;
        wy = warp.y;
        wz = warp.z;
      
        data += "," + to_string(static_cast<unsigned int>(ship.getTeam().getTeamNumber())); //[3]
        data += "," + to_string(anchorList.size()); //[4]
        for (auto& closest : anchorList)
            data += "," + closest;
        const auto nearestAnchorPos = finalAnchor->getPosition();

        px = static_cast<float>(pos.x - nearestAnchorPos.x);
        py = static_cast<float>(pos.y - nearestAnchorPos.y);
        pz = static_cast<float>(pos.z - nearestAnchorPos.z);

        qXYZ = Math::pack3NormalsInto32Int(rot.x,rot.y,rot.z);
        Math::Float16From32(&qw, rot.w);

        Math::Float16From32(&lx, lv.x);
        Math::Float16From32(&ly, lv.y);
        Math::Float16From32(&lz, lv.z);

        Math::Float16From32(&ax, av.x);
        Math::Float16From32(&ay, av.y);
        Math::Float16From32(&az, av.z);
    }
}



PacketCloakUpdate::PacketCloakUpdate() : Packet() {
    cloakTimer        = 1;
    cloakSystemOnline = false;
    cloakActive       = false;
    justTurnedOn      = false;
    justTurnedOff     = false;
}
PacketCloakUpdate::PacketCloakUpdate(Ship& ship) : PacketCloakUpdate() {
    data =        ship.getClass();  //[0]
    data += "," + ship.getMapKey(); //[1]
    if (ship.getShipSystem(ShipSystemType::CloakingDevice)) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(ship.getShipSystem(ShipSystemType::CloakingDevice));
        Math::Float16From32(&cloakTimer, cloak.getCloakTimer());
        cloakSystemOnline = cloak.isOnline();
        cloakActive       = cloak.isCloakActive();
    }
}

PacketCollisionEvent::PacketCollisionEvent() {
    damage1 = damage2 = 0.0;
    lx1 = ly1 = lz1 = ax1 = ay1 = az1 = lx2 = ly2 = lz2 = ax2 = ay2 = az2 = 0;
}
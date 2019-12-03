#include "Packet.h"

#include "PacketPhysicsUpdate.h"
#include "PacketCloakUpdate.h"
#include "PacketHealthUpdate.h"
#include "PacketProjectileImpact.h"
#include "PacketCollisionEvent.h"
#include "PacketMessage.h"
#include "PacketShipRespawned.h"
#include "PacketGameplayModeInfo.h"

using namespace std;

Packet* Packet::getPacket(const sf::Packet& sfPacket) {
    sf::Packet hardCopy = sf::Packet(sfPacket);
    Packet pp;
    pp.validate(hardCopy);
    const unsigned int packetType = pp.PacketType;
    Packet* p = nullptr;
    switch (packetType) {
        case PacketType::Server_To_Client_Accept_Connection: {
            p = new PacketMessage(); break;
        }case PacketType::Server_To_Client_Reject_Connection: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Request_Connection: {
            p = new PacketMessage(); break;
        }case PacketType::Client_To_Server_Request_Disconnection: {
            p = new PacketMessage(); break;
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
            p = new PacketMessage(); break;
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
            p = new PacketGameplayModeInfo(); break;
        }case PacketType::Server_To_Client_Request_GameplayMode: {
            p = new PacketGameplayModeInfo(); break;
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
            p = new PacketShipRespawnNotification(); break;
        }case PacketType::Server_To_Client_Notify_Ship_Of_Respawn: {
            p = new PacketShipRespawned(); break;
        }default: {
            break;
        }
    }
    if(!p)
        cout << "Invalid packet type in getPacket(), please see Packet.cpp" << endl;
    return p;
}

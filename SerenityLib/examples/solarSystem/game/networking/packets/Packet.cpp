#include "Packet.h"

#include "PacketPhysicsUpdate.h"
#include "PacketCloakUpdate.h"
#include "PacketHealthUpdate.h"
#include "PacketProjectileImpact.h"
#include "PacketCollisionEvent.h"
#include "PacketMessage.h"
#include "PacketShipRespawned.h"
#include "PacketGameplayModeInfo.h"
#include "PacketShipInfoRequest.h"

using namespace std;

Packet* Packet::getPacket(const sf::Packet& sfPacket) {
    sf::Packet hardCopy = sf::Packet(sfPacket);
    Packet packet;
    packet.validate(hardCopy);
    const unsigned int& packetType = packet.PacketType;
    switch (packetType) {
        case PacketType::Server_To_Client_Accept_Connection: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Reject_Connection: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Request_Connection: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Request_Disconnection: {
            return NEW PacketMessage();
        }case PacketType::Server_Shutdown: {
            return NEW Packet();
        }case PacketType::Client_To_Server_Ship_Physics_Update: {
            return NEW PacketPhysicsUpdate();
        }case PacketType::Server_To_Client_Ship_Physics_Update: {
            return NEW PacketPhysicsUpdate();
        }case PacketType::Client_To_Server_Chat_Message: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Chat_Message: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Joined_Server: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Left_Server: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Map_Data: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Request_Map_Entry: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Approve_Map_Entry: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Reject_Map_Entry: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_New_Client_Entered_Map: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Request_Anchor_Creation:{
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Anchor_Creation: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Successfully_Entered_Map: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Ship_Cloak_Update:{
            return NEW PacketCloakUpdate();
        }case PacketType::Server_To_Client_Ship_Cloak_Update: {
            return NEW PacketCloakUpdate();
        }case PacketType::Client_To_Server_Client_Changed_Target: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Changed_Target: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Client_Fired_Cannons:{
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Client_Fired_Beams:{
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Client_Fired_Torpedos: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Fired_Cannons: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Fired_Beams: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Fired_Torpedos: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Client_Left_Map: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Ship_Health_Update: {
            return NEW PacketHealthUpdate();
        }case PacketType::Server_To_Client_Ship_Health_Update: {
            return NEW PacketHealthUpdate();
        }case PacketType::Client_To_Server_Projectile_Cannon_Impact: {
            return NEW PacketProjectileImpact();
        }case PacketType::Client_To_Server_Projectile_Torpedo_Impact: {
            return NEW PacketProjectileImpact();
        }case PacketType::Server_To_Client_Projectile_Cannon_Impact: {
            return NEW PacketProjectileImpact();
        }case PacketType::Server_To_Client_Projectile_Torpedo_Impact: {
            return NEW PacketProjectileImpact();
        }case PacketType::Client_To_Server_Request_GameplayMode: {
            return NEW PacketGameplayModeInfo();
        }case PacketType::Server_To_Client_Request_GameplayMode: {
            return NEW PacketGameplayModeInfo();
        }case PacketType::Client_To_Server_Anti_Cloak_Status: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Anti_Cloak_Status: {
            return NEW PacketMessage();
        }case PacketType::Client_To_Server_Collision_Event: {
            return NEW PacketCollisionEvent();
        }case PacketType::Server_To_Client_Collision_Event: {
            return NEW PacketCollisionEvent();
        }case PacketType::Client_To_Server_Request_Ship_Current_Info: {
            return NEW PacketShipInfoRequest();
        }case PacketType::Server_To_Client_Request_Ship_Current_Info: {
            return NEW PacketShipInfoRequest();
        }case PacketType::Client_To_Server_Ship_Was_Just_Destroyed: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Ship_Was_Just_Destroyed: {
            return NEW PacketMessage();
        }case PacketType::Server_To_Client_Notify_Ship_Of_Impending_Respawn: {
            return NEW PacketShipRespawnNotification();
        }case PacketType::Server_To_Client_Notify_Ship_Of_Respawn: {
            return NEW PacketShipRespawned();
        }default: {
            return nullptr;
        }
        return nullptr;
    }
    return nullptr;
}

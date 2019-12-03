#include "PacketCloakUpdate.h"

#include "../../Ship.h"
#include "../../ships/shipSystems/ShipSystemCloakingDevice.h"
#include <core/engine/math/Engine_Math.h>

using namespace std;
using namespace Engine;

PacketCloakUpdate::PacketCloakUpdate() {
    cloakTimer        = 1;
    cloakSystemOnline = false;
    cloakActive       = false;
    justTurnedOn      = false;
    justTurnedOff     = false;
    ship_map_key      = "";
}
PacketCloakUpdate::PacketCloakUpdate(Ship& ship) {
    ship_map_key = ship.getMapKey();
    if (ship.getShipSystem(ShipSystemType::CloakingDevice)) {
        auto& cloak       = *static_cast<ShipSystemCloakingDevice*>(ship.getShipSystem(ShipSystemType::CloakingDevice));
        cloakSystemOnline = cloak.isOnline();
        cloakActive       = cloak.isCloakActive();
        Math::Float16From32(&cloakTimer, cloak.getCloakTimer());
    }
}
bool PacketCloakUpdate::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> cloakTimer >> cloakSystemOnline >> cloakActive >> justTurnedOn >> justTurnedOff >> ship_map_key);
}
bool PacketCloakUpdate::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << cloakTimer << cloakSystemOnline << cloakActive << justTurnedOn << justTurnedOff << ship_map_key);
}
//void PacketCloakUpdate::print() {}
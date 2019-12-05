#include "PacketHealthUpdate.h"

#include "../../Ship.h"
#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemHull.h"

using namespace std;

PacketHealthUpdate::PacketHealthUpdate() {
    currentHullHealth     = 0;
    currentShieldsHealthF = 0;
    currentShieldsHealthA = 0;
    currentShieldsHealthP = 0;
    currentShieldsHealthS = 0;
    currentShieldsHealthD = 0;
    currentShieldsHealthV = 0;
    flags                 = PacketHealthFlags::None;
    ship_class = ship_map_key = "";
}
PacketHealthUpdate::PacketHealthUpdate(Ship& ship) : PacketHealthUpdate(){
    auto* shields = static_cast<ShipSystemShields*>(ship.getShipSystem(ShipSystemType::Shields));
    auto* hull    = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));
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
    ship_class   = ship.getClass();
    ship_map_key = ship.getMapKey();
}
bool PacketHealthUpdate::validate(sf::Packet& sfPacket) {
    return (sfPacket >> PacketType >> currentHullHealth >> currentShieldsHealthF >> currentShieldsHealthA >> currentShieldsHealthP >> currentShieldsHealthS >> currentShieldsHealthD >> currentShieldsHealthV >> flags >> ship_class >> ship_map_key);
}
bool PacketHealthUpdate::build(sf::Packet& sfPacket) {
    return (sfPacket << PacketType << currentHullHealth << currentShieldsHealthF << currentShieldsHealthA << currentShieldsHealthP << currentShieldsHealthS << currentShieldsHealthD << currentShieldsHealthV << flags << ship_class << ship_map_key);
}
//void PacketHealthUpdate::print() {}
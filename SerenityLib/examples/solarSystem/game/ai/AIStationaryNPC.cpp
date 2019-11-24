#include "AIStationaryNPC.h"

#include "../Ship.h"
#include "../map/Map.h"
#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "../ships/shipSystems/ShipSystemSensors.h"

#include <iostream>

using namespace std;

AIStationaryNPC::AIStationaryNPC(Ship& ship) : AINPC(AIType::AI_Stationary){

    Map& map = static_cast<Map&>(ship.entity().scene());
    auto* sensors = static_cast<ShipSystemSensors*>(ship.getShipSystem(ShipSystemType::Sensors));
    auto* weapons = static_cast<ShipSystemWeapons*>(ship.getShipSystem(ShipSystemType::Weapons));

    if (weapons && sensors) {
        installGunneryLogic(m_Type, *m_Threat, ship, map, *sensors, *weapons);
    }else{
        cout << "Stationary AI NPC does not have gunnery logic\n";
    }
}
AIStationaryNPC::~AIStationaryNPC() {

}

void AIStationaryNPC::update(const double& dt) {
    AINPC::update(dt);
}
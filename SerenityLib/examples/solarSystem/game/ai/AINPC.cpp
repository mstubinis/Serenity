#include "AINPC.h"
#include "logic/GunneryBaseClass.h"
#include "ThreatTable.h"
#include <core/engine/utils/Utils.h>

AINPC::AINPC(const AIType::Type type) : AI(type) {
    m_Gunnery = nullptr;
}
AINPC::~AINPC() {
    SAFE_DELETE(m_Gunnery);
}

void AINPC::update(const double& dt) {
    if (m_Gunnery)
        m_Gunnery->update(dt);
    AI::update(dt);
}
GunneryBaseClass* AINPC::getGunneryLogic() {
    return m_Gunnery;
}
void AINPC::installGunneryLogic(AIType::Type& type, ThreatTable& table, Ship& ship, Map& map, ShipSystemSensors& sensors, ShipSystemWeapons& weapons) {
    SAFE_DELETE(m_Gunnery);
    //TODO: switch(type) and install gunnery derived classes
    m_Gunnery = new GunneryBaseClass(type, *this, table, ship, map, sensors, weapons);
}
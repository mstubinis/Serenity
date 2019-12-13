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
void AINPC::installGunneryLogic(Ship& ship, Map& map, ShipSystemSensors& sensors, ShipSystemWeapons& weapons) {
    SAFE_DELETE(m_Gunnery);
    //TODO: switch(type) and install gunnery derived classes
    m_Gunnery = NEW GunneryBaseClass(m_Type, *this, *m_Threat, ship, map, sensors, weapons);
}
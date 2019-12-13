#include "AI.h"
#include "FireAtWill.h"
#include "ThreatTable.h"
#include <core/engine/utils/Utils.h>

using namespace std;

AI::AI(const AIType::Type type) {
    m_Type         = type;
    m_FireAtWill   = nullptr;
    m_Threat       = nullptr;
}
AI::~AI() {
    SAFE_DELETE(m_FireAtWill);
    SAFE_DELETE(m_Threat);
}

const AIType::Type& AI::getType() const {
    return m_Type;
}

void AI::update(const double& dt) {
    if (m_FireAtWill) {
        m_FireAtWill->update(dt);
    }
}
void AI::installFireAtWill(AIType::Type& type, Ship& ship, Map& map, ShipSystemSensors& sensors, ShipSystemWeapons& weapons) {
    SAFE_DELETE(m_FireAtWill);
    m_FireAtWill = NEW FireAtWill(type, ship, map, sensors, weapons);
}
void AI::installThreatTable(Map& map) {
    SAFE_DELETE(m_Threat);
    m_Threat = NEW ThreatTable(map);
}
FireAtWill* AI::getFireAtWill() {
    return m_FireAtWill;
}
ThreatTable* AI::getThreatTable() {
    return m_Threat;
}

/*
const string AI::serialize() const {
    string res = "";
    
    res += to_string(m_Type);

    if (m_FireAtWill) {
        res += "1";
    }else{
        res += "0";
    }
    return res;
}
void AI::deserialize(const string& input) {

}
*/
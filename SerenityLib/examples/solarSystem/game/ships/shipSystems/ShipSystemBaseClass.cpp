#include "ShipSystemBaseClass.h"

ShipSystem::ShipSystem(const uint& _type, Ship& _ship) :m_Ship(_ship) {
    m_Health = 1.0f;
    m_Power = 1.0f;
    m_Type = _type;
}
ShipSystem::~ShipSystem() {
    m_Health = 0.0f;
    m_Power = 0.0f;
}
const bool ShipSystem::isOnline() const {
    return (m_Health > 0 && m_Power > 0) ? true : false; 
}
void ShipSystem::update(const double& dt) {
    // handle power transfers...?
}
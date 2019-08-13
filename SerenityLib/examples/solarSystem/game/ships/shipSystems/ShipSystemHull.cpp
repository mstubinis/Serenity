#include "ShipSystemHull.h"
#include "../../Ship.h"

ShipSystemHull::ShipSystemHull(Ship& _ship, const uint health) :ShipSystem(ShipSystemType::Hull, _ship) {
    m_HealthPointsCurrent = m_HealthPointsMax = health;

    m_RechargeAmount = 50;
    m_RechargeRate = 5.0f;
    m_RechargeTimer = 0.0f;

    _ship.getComponent<ComponentBody>()->setUserPointer(this);
}
ShipSystemHull::~ShipSystemHull() {

}
void ShipSystemHull::receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const uint damage) {
    int newHP = m_HealthPointsCurrent - damage;
    if (newHP > 0) {
        //hull takes entire hit
        m_HealthPointsCurrent -= damage;
    }else{
        //we destroyed the ship
        m_HealthPointsCurrent = 0;
    }
}
void ShipSystemHull::update(const double& dt) {

    const float fdt = static_cast<float>(dt);
#pragma region Recharging
    if (m_HealthPointsCurrent < m_HealthPointsMax) { //dont need to recharge at max shields
        m_RechargeTimer += fdt;
        if (m_RechargeTimer >= m_RechargeRate) {
            m_HealthPointsCurrent += m_RechargeAmount;
            if (m_HealthPointsCurrent > m_HealthPointsMax) {
                m_HealthPointsCurrent = m_HealthPointsMax;
            }
            m_RechargeTimer = 0.0f;
        }
    }
#pragma endregion

    ShipSystem::update(dt);
}
const uint ShipSystemHull::getHealthCurrent() const {
    return m_HealthPointsCurrent;
}
const uint ShipSystemHull::getHealthMax() const {
    return m_HealthPointsMax;
}
const float ShipSystemHull::getHealthPercent() const {
    return static_cast<float>(m_HealthPointsCurrent) / static_cast<float>(m_HealthPointsMax);
}
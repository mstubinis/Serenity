#include "ShipSystemHull.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ResourceManifest.h"

#include <core/engine/renderer/Decal.h>

#include <ecs/Components.h>


ShipSystemHull::ShipSystemHull(Ship& _ship, Map& map, const uint health) :ShipSystem(ShipSystemType::Hull, _ship), m_HullEntity(map), m_Map(map){
    m_HealthPointsCurrent = m_HealthPointsMax = health;

    m_RechargeAmount = 50;
    m_RechargeRate = 5.0f;
    m_RechargeTimer = 0.0f;
    m_CollisionTimer = 10.0f;

    auto& hullBody = *m_HullEntity.addComponent<ComponentBody>(CollisionType::TriangleShapeStatic);
    auto col = new Collision(CollisionType::TriangleShapeStatic, _ship.getComponent<ComponentModel>()->getModel().mesh(), _ship.getComponent<ComponentBody>()->mass());
    hullBody.setCollision(col);
    hullBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    hullBody.setCollisionGroup(CollisionFilter::_Custom_3); //group 3 are hull
    hullBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons

    hullBody.setUserPointer(this);
    hullBody.setUserPointer1(&_ship);
}
ShipSystemHull::~ShipSystemHull() {

}
void ShipSystemHull::receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const float& maxTime, const uint damage) {
    int newHP = m_HealthPointsCurrent - damage;
    if (newHP > 0) {
        //hull takes entire hit
        m_HealthPointsCurrent -= damage;
    }else{
        //we destroyed the ship
        m_HealthPointsCurrent = 0;
    }

    //add damage decal
    auto& hullDamage1 = *(Material*)ResourceManifest::HullDamageMaterial1.get();
    Decal* d = new Decal(hullDamage1, impactLocationLocal * m_Ship.getRotation(), impactNormal, impactRadius * 0.4f, m_Map, 120);
    m_Ship.m_DamageDecals.push_back(d);
}
void ShipSystemHull::receiveCollision(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const float damage) {
    if (m_CollisionTimer > static_cast<float>(HULL_TO_HULL_COLLISION_DELAY)) {
        auto damageInt = static_cast<int>(damage);
        int newHP = m_HealthPointsCurrent - damageInt;
        if (newHP > 0) {
            //hull takes entire hit
            m_HealthPointsCurrent -= damageInt;
        }else{
            //we destroyed the ship
            m_HealthPointsCurrent = 0;
        }
        m_CollisionTimer = 0.0f;


        //add damage decal
        auto& hullDamage1 = *(Material*)ResourceManifest::HullDamageMaterial1.get();
        Decal* d = new Decal(hullDamage1, impactLocationLocal * m_Ship.getRotation(), impactNormal, impactRadius * 0.4f, m_Map, 120);
        m_Ship.m_DamageDecals.push_back(d);
    }
}
void ShipSystemHull::update(const double& dt) {
    auto& hullBody = *m_HullEntity.getComponent<ComponentBody>();
    auto& shipBody = *m_Ship.getComponent<ComponentBody>();
    hullBody.setPosition(shipBody.position());
    hullBody.setRotation(shipBody.rotation());


    const float fdt = static_cast<float>(dt);
    if (m_CollisionTimer < 10.0f + static_cast<float>(HULL_TO_HULL_COLLISION_DELAY)) {
        m_CollisionTimer += fdt;
    }

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
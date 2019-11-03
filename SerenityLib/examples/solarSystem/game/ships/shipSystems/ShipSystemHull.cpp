#include "ShipSystemHull.h"
#include "ShipSystemCloakingDevice.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ResourceManifest.h"
#include "../../Helper.h"

#include <core/engine/renderer/Decal.h>

#include <ecs/Components.h>
#include <glm/gtx/norm.hpp>

using namespace std;

ShipSystemHull::ShipSystemHull(Ship& _ship, Map& map, const float health) :ShipSystem(ShipSystemType::Hull, _ship), m_Map(map){
    m_HullEntity = map.createEntity();
    m_HealthPointsCurrent = m_HealthPointsMax = health;

    m_RechargeAmount = 50.0f;
    m_RechargeRate = 5.0f;
    m_RechargeTimer = 0.0f;
    m_CollisionTimer = 10.0f;

    auto& hullBody = *m_HullEntity.addComponent<ComponentBody>(CollisionType::TriangleShapeStatic);
    auto* col = new Collision(&hullBody, *_ship.getComponent<ComponentModel>(), _ship.getComponent<ComponentBody>()->mass(), CollisionType::TriangleShapeStatic);
    hullBody.setCollision(col);
    hullBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    hullBody.setCollisionGroup(CollisionFilter::_Custom_3); //group 3 are hull
    hullBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons

    hullBody.setUserPointer(this);
    hullBody.setUserPointer1(&_ship);
    hullBody.setUserPointer2(&_ship);
}
ShipSystemHull::~ShipSystemHull() {

}
Entity ShipSystemHull::getEntity() {
    return m_HullEntity;
}
void ShipSystemHull::destroy() {
    m_HullEntity.destroy();
    m_RechargeTimer = 0.0f;
    m_CollisionTimer = 10.0f;
}
void ShipSystemHull::applyDamageDecal(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const bool forceHullFire) {
    auto& decalList = m_Ship.m_DamageDecals;
    Decal* d = nullptr;
    Decal* d1 = nullptr;

    const auto rand = Helper::GetRandomFloatFromTo(0.0f, 1.0f);
    Material* hullDamageOutline = nullptr;
    Material* hullDamage = nullptr;
    if (rand > 0.0f && rand <= 0.33333f) {
        hullDamageOutline = (Material*)ResourceManifest::HullDamageOutline1Material.get();
        hullDamage = (Material*)ResourceManifest::HullDamageMaterial1.get();
    }else if (rand > 0.333333f && rand <= 0.666666f){
        hullDamageOutline = (Material*)ResourceManifest::HullDamageOutline2Material.get();
        hullDamage = (Material*)ResourceManifest::HullDamageMaterial2.get();
    }else{
        hullDamageOutline = (Material*)ResourceManifest::HullDamageOutline3Material.get();
        hullDamage = (Material*)ResourceManifest::HullDamageMaterial3.get();
    }

    const auto impactLocation = glm_vec3(impactLocationLocal) * m_Ship.getRotation();
    const auto impactR        = impactRadius * 0.16f;
    auto*      cloakingDevice = static_cast<ShipSystemCloakingDevice*>(m_Ship.getShipSystem(ShipSystemType::CloakingDevice));
    if (forceHullFire) {
        d = new Decal(*hullDamageOutline, impactLocation, impactNormal, impactR, m_Map, 120);
        decalList.push_back(d);
        d1 = new Decal(*hullDamage, impactLocation, impactNormal, impactR, m_Map, 120, RenderStage::Decals_2);
        decalList.push_back(d1);
        if (cloakingDevice) m_Ship.updateDamageDecalsCloak(glm::abs(cloakingDevice->getCloakTimer()));
        return;
    }

    if (decalList.size() == 0) {
        d = new Decal(*hullDamageOutline, impactLocation, impactNormal, impactR, m_Map, 120);
        decalList.push_back(d);
        if (cloakingDevice) m_Ship.updateDamageDecalsCloak(glm::abs(cloakingDevice->getCloakTimer()));
        return;
    }else{
        //get list of nearby impact points
        vector<Decal*> nearbys;
        for (auto& dec : decalList) {
            auto dist = glm::distance2(glm_vec3(dec->initialPosition()), impactLocation);
            if (dist < (impactR * impactR) * 0.4f) {
                nearbys.push_back(dec);
            }
        }
        if (nearbys.size() >= 8) {
            return; //forget it, no need to have so many
        }

        d = new Decal(*hullDamageOutline, impactLocation, impactNormal, impactR, m_Map, 120);
        if (nearbys.size() >= 4) {
            d1 = new Decal(*hullDamage, impactLocation, impactNormal, impactR, m_Map, 120, RenderStage::Decals_2);
        }
    }
    if (d)  
        decalList.push_back(d);
    if (d1) 
        decalList.push_back(d1);
    if(cloakingDevice) m_Ship.updateDamageDecalsCloak(glm::abs(cloakingDevice->getCloakTimer()));
}
void ShipSystemHull::receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const float& maxTime, const float damage, const bool forceHullFire, const bool paint) {
    float newHP = m_HealthPointsCurrent - damage;
    if (newHP > 0.0f) {
        //hull takes entire hit
        m_HealthPointsCurrent -= damage;
    }else{
        //we destroyed the ship
        m_HealthPointsCurrent = 0.0f;
    }
    if(paint)
        applyDamageDecal(impactNormal, impactLocationLocal, impactRadius, forceHullFire);
}
void ShipSystemHull::receiveCollision(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const float damage) {
    if (m_CollisionTimer > static_cast<float>(HULL_TO_HULL_COLLISION_DELAY)) {
        float newHP = m_HealthPointsCurrent - damage;
        if (newHP > 0.0f) {
            //hull takes entire hit
            m_HealthPointsCurrent -= damage;
        }else{
            //we destroyed the ship
            m_HealthPointsCurrent = 0.0f;
        }
        m_CollisionTimer = 0.0f;
        applyDamageDecal(impactNormal, impactLocationLocal, impactRadius, true);
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
const float ShipSystemHull::getHealthCurrent() const {
    return m_HealthPointsCurrent;
}
const float ShipSystemHull::getHealthMax() const {
    return m_HealthPointsMax;
}
const float ShipSystemHull::getHealthPercent() const {
    return (m_HealthPointsCurrent) / (m_HealthPointsMax);
}
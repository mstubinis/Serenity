#include "ShipSystemHull.h"
#include "ShipSystemCloakingDevice.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ResourceManifest.h"
#include "../../Helper.h"
#include "../../particles/Fire.h"

#include <core/engine/renderer/Decal.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>

#include <ecs/Components.h>
#include <glm/gtx/norm.hpp>

using namespace std;

ShipSystemHull::ShipSystemHull(Ship& _ship, Map& map, const float health, const float recharge_amount, const float recharge_rate) :ShipSystem(ShipSystemType::Hull, _ship), m_Map(map){
    m_HullEntity = map.createEntity();
    m_HealthPointsCurrent = m_HealthPointsMax = health;

    m_RechargeAmount = recharge_amount;
    m_RechargeRate = recharge_rate;
    m_RechargeTimer = 0.0f;

    auto& hullBody = *m_HullEntity.addComponent<ComponentBody>(CollisionType::TriangleShapeStatic);
    auto* col = new Collision(&hullBody, *_ship.getComponent<ComponentModel>(), _ship.getComponent<ComponentBody>()->mass(), CollisionType::TriangleShapeStatic);
    hullBody.setCollision(col);
    hullBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    hullBody.setCollisionGroup(CollisionFilter::_Custom_3); //group 3 are hull
    hullBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons

    hullBody.setUserPointer(this);
    hullBody.setUserPointer1(&_ship);
    hullBody.setUserPointer2(&_ship);

    auto& btBody = const_cast<btRigidBody&>(hullBody.getBtBody());
    btBody.setCollisionFlags(btBody.getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}
ShipSystemHull::~ShipSystemHull() {

}
void ShipSystemHull::restoreToFull() {
    m_HealthPointsCurrent = m_HealthPointsMax;
}
Entity ShipSystemHull::getEntity() {
    return m_HullEntity;
}
void ShipSystemHull::destroy() {
    m_HullEntity.destroy();
    m_RechargeTimer = 0.0f;
}
void ShipSystemHull::applyDamageDecal(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const size_t modelIndex, const bool forceHullFire) {
    std::cout << modelIndex << "\n";
    auto shipRotation = m_Ship.getRotation();
    auto lamda_apply_random_emitter = [&]() {
        auto rand = Helper::GetRandomIntFromTo(0, 100);
        if (rand < 85) {
            auto lifetime = Helper::GetRandomFloatFromTo(32.75f, 46.0f);

            auto finalPos = m_Ship.getPosition() + Engine::Math::rotate_vec3(shipRotation, impactLocationLocal);

            ParticleEmitter emitter_(*Fire::ShortLived, m_Map, lifetime);
            EntityDataRequest req(emitter_.entity());

            glm_quat q;
            Engine::Math::alignTo(q, -impactNormal);

            emitter_.setPosition(finalPos, req);
            emitter_.setRotation(q);

            auto* emitter = m_Map.addParticleEmitter(emitter_);
            if (emitter) {
                m_Ship.m_EmittersDestruction.push_back(std::make_tuple(emitter, modelIndex, impactLocationLocal, q));
            }
        }
    };



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

    const auto impactR        = impactRadius * 0.16f;
    auto*      cloakingDevice = static_cast<ShipSystemCloakingDevice*>(m_Ship.getShipSystem(ShipSystemType::CloakingDevice));
    auto&      model          = *m_Ship.getComponent<ComponentModel>();
    if (forceHullFire) {
        d = new Decal(*hullDamageOutline, impactLocationLocal, impactNormal, impactR, m_Map, 120);
        decalList.push_back(d);
        d1 = new Decal(*hullDamage, impactLocationLocal, impactNormal, impactR, m_Map, 120, RenderStage::Decals_2);
        decalList.push_back(d1);

        lamda_apply_random_emitter();

        if (cloakingDevice) {
            m_Ship.updateCloakVisuals(glm::abs(cloakingDevice->getCloakTimer()), model);
        }
        return;
    }
    if (decalList.size() == 0) {
        d = new Decal(*hullDamageOutline, impactLocationLocal, impactNormal, impactR, m_Map, 120);
        decalList.push_back(d);
        if (cloakingDevice) {
            m_Ship.updateCloakVisuals(glm::abs(cloakingDevice->getCloakTimer()), model);
        }
        return;
    }else{
        //get list of nearby impact points
        vector<Decal*> nearbys;
        for (auto& dec : decalList) {
            decimal dist = glm::distance2(glm_vec3(dec->initialPosition()), glm_vec3(impactLocationLocal));
            if (dist < static_cast<decimal>(impactR * impactR) * static_cast<decimal>(0.4f)) {
                nearbys.push_back(dec);
            }
        }
        if (nearbys.size() >= 8) {
            return; //forget it, no need to have so many
        }
        d = new Decal(*hullDamageOutline, impactLocationLocal, impactNormal, impactR, m_Map, 120);
        if (nearbys.size() >= 4) {
            d1 = new Decal(*hullDamage, impactLocationLocal, impactNormal, impactR, m_Map, 120, RenderStage::Decals_2);
        }
    }
    if (d) {
        decalList.push_back(d);
    }
    if (d1) {
        decalList.push_back(d1);
        lamda_apply_random_emitter();
    }
    if (cloakingDevice) {
        m_Ship.updateCloakVisuals(glm::abs(cloakingDevice->getCloakTimer()), model);
    }
}
void ShipSystemHull::receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactModelSpacePosition, const float& impactRadius, const float damage, const size_t modelIndex, const bool forceHullFire, const bool paint) {
    receiveCollisionDamage(damage);
    if (paint) {
        applyDamageDecal(impactNormal, impactModelSpacePosition, impactRadius, modelIndex, forceHullFire);
    }
}
void ShipSystemHull::receiveCollisionDamage(const float damage) {
    const float newHP = m_HealthPointsCurrent - damage;
    if (newHP > 0.0f) {
        //hull takes entire hit
        m_HealthPointsCurrent -= damage;
    }else{
        //we destroyed the ship
        m_HealthPointsCurrent = 0.0f;
        if (!m_Ship.isDestroyed()) {
            m_Ship.setState(ShipState::JustFlaggedForDestruction);
        }
    }
}
void ShipSystemHull::receiveCollisionVisual(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const size_t modelIndex) {
    applyDamageDecal(impactNormal, impactLocationLocal, impactRadius, modelIndex, true);
}

void ShipSystemHull::update(const double& dt) {
    auto& hullBody = *m_HullEntity.getComponent<ComponentBody>();
    auto& shipBody = *m_Ship.getComponent<ComponentBody>();
    hullBody.setPosition(shipBody.position());
    hullBody.setRotation(shipBody.rotation());

#pragma region Recharging
    if (m_Ship.m_State == ShipState::Nominal) {
        const auto fdt = static_cast<float>(dt);
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
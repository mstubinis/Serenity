#include "ShipSystemShields.h"
#include "ShipSystemHull.h"
#include "ShipSystemSensors.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ResourceManifest.h"
#include "../../Helper.h"
#include "../../ships/Ships.h"

#include <core/engine/model/ModelInstance.h>
#include <ecs/Components.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/physics/Collision.h>

#include <glm/gtc/matrix_transform.hpp>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

using namespace Engine;
using namespace std;

#pragma region Plane

ShipSystemShields::Plane::Plane() {
    a = b = c = normal = glm::vec3(0.0f);
}
ShipSystemShields::Plane::Plane(const glm::vec3& _a, const glm::vec3& _b, const glm::vec3& _c) {
    a = _a;
    b = _b;
    c = _c;
    normal = glm::normalize(glm::cross(b - a, c - a));
}
const float ShipSystemShields::Plane::CalcSide(const glm::vec3& impactLocationModelSpace) {
    return glm::dot(impactLocationModelSpace, normal);
}

#pragma endregion

#pragma region Pyramid
ShipSystemShields::Pyramid::Pyramid() {
}
ShipSystemShields::Pyramid::Pyramid(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& D, const glm::vec3& E) { 
    ShipSystemShields::Plane left(E, C, A);//E,C,A
    ShipSystemShields::Plane right(E, B, D);//E,B,D
    ShipSystemShields::Plane top(E, A, B);//E,A,B
    ShipSystemShields::Plane bottom(E, D, C);//E,D,C
    ShipSystemShields::Plane frontTopLeft(A, B, D);//A,B,C
    ShipSystemShields::Plane frontBottomRight(D, C, A);//B,D,C

    planes.resize(6);

    planes[0] = left;
    planes[1] = right;
    planes[2] = top;
    planes[3] = bottom;
    planes[4] = frontTopLeft;
    planes[5] = frontBottomRight;
}
const bool ShipSystemShields::Pyramid::isInside(const glm::vec3& impactLocationModelSpace) {
    for (auto& plane : planes) {
        const float res = plane.CalcSide(impactLocationModelSpace);
        if (res > 0.0f) {
            return false;
        }
    }
    return true;
}

#pragma endregion


struct ShieldInstanceBindFunctor {void operator()(EngineResource* r) const {
    Renderer::GLDisable(GL_CULL_FACE);

    auto& i = *static_cast<ModelInstance*>(r);
    auto& shields = *static_cast<ShipSystemShields*>(i.getUserPointer());
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto bodypos = glm::vec3(body.position());
    glm::mat4 parentModel = body.modelMatrixRendering();
    auto& model = *parent.getComponent<ComponentModel>();
    const float shipScale = Math::Max(glm::vec3(shields.m_Ship.getScale()));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    glm::mat4 modelMatrix = parentModel * i.modelMatrix();

    /*
    for some reason, body.modelMatrix() is outputing perfect world positioning during the rendering stage, but the bullet rigid body (or more specifically, maybe it's collision position)
    it not outputting the updated positioning, gotta dumb it down here...
    */
    modelMatrix[3][0] = bodypos.x;
    modelMatrix[3][1] = bodypos.y;
    modelMatrix[3][2] = bodypos.z;

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));


    Renderer::sendUniform1("uv_scale", glm::round((model.radius() *  shipScale) * 0.4f));
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
    int count = 0;
    const glm::vec3 finalModelPosition = Math::getMatrixPosition(modelMatrix);
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        auto& impact = shields.m_ImpactPoints[i];
        if (impact.active) {
            glm::vec3 worldPosition = finalModelPosition + glm::vec3(Math::rotate_vec3(body.rotation(), impact.impactLocation));
            Renderer::sendUniform3Safe(("impacts[" + to_string(count) + "].Position").c_str(), worldPosition);
            Renderer::sendUniform3Safe(("impacts[" + to_string(count) + "].Data").c_str(), impact.impactRadius, impact.currentTime, impact.maxTime);
            ++count;
        }
    }
    Renderer::sendUniform1Safe("numImpacts", count);
}};
struct ShieldInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    Renderer::GLEnable(GL_CULL_FACE);
}};

#pragma region ShieldImpactStruct
ShipSystemShieldsImpactPoint::ShipSystemShieldsImpactPoint() {
    reset(0);
}
ShipSystemShieldsImpactPoint::ShipSystemShieldsImpactPoint(const unsigned int& index) {
    reset(index);
}
void ShipSystemShieldsImpactPoint::impact(const glm::vec3& _impactLocation, const float& _impactRadius, const float& _maxTime, vector<uint>& freelist) {
    active = true;
    impactLocation = _impactLocation;
    impactRadius = _impactRadius;
    currentTime = 0.0f;
    maxTime = _maxTime;

    removeFromVector(freelist, indexInArray);
}
void ShipSystemShieldsImpactPoint::reset(const unsigned int& index) {
    active = false;
    impactLocation = glm::vec3(0.0f);
    impactRadius = maxTime = currentTime = 0.0f;
    indexInArray = index;
}
const bool ShipSystemShieldsImpactPoint::update(const float& dt, vector<uint>& freelist, size_t& maxIndex) {
    if (active) {
        currentTime += dt;
        if (currentTime >= maxTime) {
            active = false;
            currentTime = 0.0f;
            freelist.push_back(indexInArray);

            if (indexInArray >= maxIndex) {
                //this was the biggest index, we need a new one
                return true;
            }
        }
    }
    return false;
}
#pragma endregion

ShipSystemShields::ShipSystemShields(Ship& _ship, Map& map, float health, glm::vec3 offset, glm::vec3 additional_size_scale, float recharge_amount) :ShipSystemShields(_ship, map, health / 6.0f, health / 6.0f, health / 6.0f, health / 6.0f, health / 6.0f, health / 6.0f, offset, additional_size_scale, recharge_amount) {

}
ShipSystemShields::ShipSystemShields(Ship& _ship, Map& map, float fwd, float aft, float prt, float sbd, float dsl, float vnt, glm::vec3 offset, glm::vec3 additional_size_scale, float recharge_amount) :ShipSystem(ShipSystemType::Shields, _ship){
    m_ShieldEntity = map.createEntity();
    m_ShieldsOffset = offset;
    m_AdditionalShieldScale = additional_size_scale;
    auto& model = *m_ShieldEntity.addComponent<ComponentModel>(ResourceManifest::ShieldMesh, ResourceManifest::ShieldMaterial, ResourceManifest::shieldsShaderProgram, RenderStage::ForwardParticles);

    //TODO: optimize collision hull if possible
    auto& shieldBody = *m_ShieldEntity.addComponent<ComponentBody>(CollisionType::TriangleShapeStatic);
    shieldBody.setUserPointer(this);
    shieldBody.setUserPointer1(&_ship);
    shieldBody.setUserPointer2(&_ship);
    Mesh& shieldColMesh = *(Mesh*)ResourceManifest::ShieldColMesh.get();
    Collision* c = NEW Collision(CollisionType::TriangleShapeStatic, shieldColMesh, shieldBody.mass());
    c->getBtShape()->setMargin(0.08f);
    shieldBody.setCollision(c);

    m_HealthPointsMax.reserve(6);
    m_HealthPointsCurrent.reserve(6);

    m_HealthPointsCurrent.push_back(fwd);
    m_HealthPointsCurrent.push_back(aft);
    m_HealthPointsCurrent.push_back(prt);
    m_HealthPointsCurrent.push_back(sbd);
    m_HealthPointsCurrent.push_back(dsl);
    m_HealthPointsCurrent.push_back(vnt);

    m_HealthPointsMax.push_back(fwd);
    m_HealthPointsMax.push_back(aft);
    m_HealthPointsMax.push_back(prt);
    m_HealthPointsMax.push_back(sbd);
    m_HealthPointsMax.push_back(dsl);
    m_HealthPointsMax.push_back(vnt);

    m_RechargeAmount = recharge_amount;
    m_RechargeRate = 15.0f;
    m_RechargeTimer = 0.0f;

    auto shieldScale = m_Ship.getComponent<ComponentModel>()->boundingBox() * SHIELD_SCALE_FACTOR;
    shieldScale *= m_AdditionalShieldScale;

    auto& btBody = const_cast<btRigidBody&>(shieldBody.getBtBody());
    shieldBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    shieldBody.setCollisionGroup(CollisionFilter::_Custom_1); //group 1 are shields
    shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons

    btBody.setUserPointer(&m_Ship);
    shieldBody.setScale(shieldScale);

    auto& shipInfo = Ships::Database.at(m_Ship.getClass());
    auto& shieldColor = shipInfo.FactionInformation.ColorShield;
    auto& instance = model.getModel(0);
    instance.setColor(shieldColor.r, shieldColor.g, shieldColor.b, shieldColor.a);
    instance.setCustomBindFunctor(ShieldInstanceBindFunctor());
    instance.setCustomUnbindFunctor(ShieldInstanceUnbindFunctor());
    instance.setUserPointer(this);

    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        m_ImpactPoints[i].indexInArray = i;
    }
    m_ImpactPointsFreelist.reserve(MAX_IMPACT_POINTS);
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        m_ImpactPointsFreelist.push_back(i);
    }
    m_MaxIndex = 0;
    m_ShieldsAreUp = true;


    //TODO: create shield pyramids (6 total) here
    const auto& x = shieldScale.x;
    const auto& y = shieldScale.y;
    const auto& z = shieldScale.z;
    glm::vec3 A, B, C, D, E, F, G, H, I;

    A = glm::vec3(-x,  y, -z);
    B = glm::vec3( x,  y, -z);
    C = glm::vec3(-x, -y, -z);
    D = glm::vec3( x, -y, -z);
    E = glm::vec3( 0,  0,  0);
    F = glm::vec3(-x,  y,  z);
    G = glm::vec3( x,  y,  z);
    H = glm::vec3(-x, -y,  z);
    I = glm::vec3( x, -y,  z);

    ShipSystemShields::Pyramid front(G, F, I, H, E);
    ShipSystemShields::Pyramid back(A, B, C, D, E);
    ShipSystemShields::Pyramid left(B, G, D, I, E);
    ShipSystemShields::Pyramid right(F, A, H, C, E);
    ShipSystemShields::Pyramid top(A, B, F, G, E);
    //ShipSystemShields::Pyramid bottom(C, D, H, I, E);

    m_Pyramids.resize(5);

    m_Pyramids[0] = front;
    m_Pyramids[1] = back;
    m_Pyramids[2] = left;
    m_Pyramids[3] = right;
    m_Pyramids[4] = top;
    //m_Pyramids[5] = bottom;
}
ShipSystemShields::~ShipSystemShields() {
}
void ShipSystemShields::destroy() {
    m_ShieldEntity.destroy();
    m_ShieldsAreUp = false;
    reset_all_impact_points();
}

const bool ShipSystemShields::isPointInside(const glm::vec3& ptInModelSpace) {
    const glm::vec3 radi = getEntity().getComponent<ComponentBody>()->getScale();
    const auto x1 = (ptInModelSpace.x) * (ptInModelSpace.x);
    const auto y1 = (ptInModelSpace.y) * (ptInModelSpace.y);
    const auto z1 = (ptInModelSpace.z) * (ptInModelSpace.z);
    const auto res = (x1 / (radi.x * radi.x)) + (y1 / (radi.z * radi.z)) + (z1 / (radi.y * radi.y));
    return (res < 1.0f) ? true : false;
}

void ShipSystemShields::restoreToFull() {
    for (size_t i = 0; i < m_HealthPointsCurrent.size(); ++i) {
        m_HealthPointsCurrent[i] = m_HealthPointsMax[i];
    }
    m_MaxIndex = 0;
}
void ShipSystemShields::reset_all_impact_points() {
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        m_ImpactPoints[i].reset(i);
    }
    m_MaxIndex = 0;
}
void ShipSystemShields::update(const double& dt) {
    const float fdt = static_cast<float>(dt);

    auto& shieldBody        = *m_ShieldEntity.getComponent<ComponentBody>();
    auto& shieldModel       = *m_ShieldEntity.getComponent<ComponentModel>();
    auto& shipBody          = *m_Ship.getComponent<ComponentBody>();
    const auto shipBodyRot  = shipBody.rotation();

    shieldBody.setRotation(shipBodyRot);
    shieldBody.setPosition(shipBody.position() + Math::rotate_vec3(shipBodyRot, shieldModel.getModel(0).position() + m_ShieldsOffset));
    
    bool shown            = false;
    bool recalc_max_index = false;
    for (size_t i = 0; i <= m_MaxIndex; ++i) {
        auto& impact = m_ImpactPoints[i];
        recalc_max_index = impact.update(fdt, m_ImpactPointsFreelist, m_MaxIndex);
        if (!shown && impact.active) {
            shown = true;
        }
    }
    if (recalc_max_index) { //we need a new max index
        for (int j = MAX_IMPACT_POINTS - 1; j >= 0; --j) {
            auto& impact1 = m_ImpactPoints[j];
            if (impact1.active) {
                m_MaxIndex = j;
                break;
            }
        }
    }
    !shown ? shieldModel.hide() : shieldModel.show();

    //recharging here
    #pragma region Recharging
    if (m_Ship.m_State == ShipState::Nominal) {
        bool shouldRecharge = false;
        for (uint i = 0; i < m_HealthPointsCurrent.size(); ++i) {
            if (m_HealthPointsCurrent[i] < m_HealthPointsMax[i]) { //dont need to recharge at max shields
                shouldRecharge = true;
                break;
            }
        }
        if (shouldRecharge) {
            m_RechargeTimer += fdt;
            if (m_RechargeTimer >= m_RechargeRate) {
                for (uint i = 0; i < m_HealthPointsCurrent.size(); ++i) {
                    if (m_HealthPointsCurrent[i] < m_HealthPointsMax[i]) { //dont need to recharge at max shields
                        m_HealthPointsCurrent[i] += m_RechargeAmount;
                        if (m_HealthPointsCurrent[i] > m_HealthPointsMax[i]) {
                            m_HealthPointsCurrent[i] = m_HealthPointsMax[i];
                        }
                    }
                }
                m_RechargeTimer = 0.0f;
            }
        }
    }
    #pragma endregion

    ShipSystem::update(dt);
}
const glm::vec3& ShipSystemShields::getAdditionalShieldSizeScale() const {
    return m_AdditionalShieldScale;
}
ShipSystemShields::ShieldSide::Side ShipSystemShields::getImpactSide(const glm::vec3& impactLocationModelSpace) {
    for (uint i = 0; i < m_Pyramids.size(); ++i) {
        const auto res = m_Pyramids[i].isInside(impactLocationModelSpace);
        if (res == true) {
            return static_cast<ShipSystemShields::ShieldSide::Side>(i);
        }
    }
    return ShipSystemShields::ShieldSide::Ventral;
}
const string ShipSystemShields::getImpactSideString(const ShieldSide::Side side) {
    if (side == ShieldSide::Side::Forward)
        return "Forward";
    if (side == ShieldSide::Side::Aft)
        return "Aft";
    if (side == ShieldSide::Side::Port)
        return "Port";
    if (side == ShieldSide::Side::Starboard)
        return "Starboard";
    if (side == ShieldSide::Side::Dorsal)
        return "Dorsal";
    if (side == ShieldSide::Side::Ventral)
        return "Ventral";
    if (side == ShieldSide::Side::Err)
        return "Error";
    return "Error";
}

void ShipSystemShields::addShieldImpact(const glm::vec3& impactModelSpacePosition, const float& impactRadius, const float& maxTime) {
    if (m_ImpactPointsFreelist.size() > 0) {
        auto nextIndex = m_ImpactPointsFreelist[0];
        auto& impactPointData = m_ImpactPoints[nextIndex];
        impactPointData.impact(impactModelSpacePosition, impactRadius, maxTime, m_ImpactPointsFreelist);
        if (nextIndex > m_MaxIndex) {
            m_MaxIndex = nextIndex;
        }
    }
}
void ShipSystemShields::receiveHit(const string& source, const glm::vec3& impactNormal, const glm::vec3& impactModelSpacePosition, const float& impactRadius, const float& maxTime, const float damage, const uint shieldSide, const bool doImpactGraphic) {
    if (m_ShieldsAreUp) {
        if (m_Ship.m_State != ShipState::Destroyed && m_Ship.m_State != ShipState::JustFlaggedAsFullyDestroyed) {
            /*
            rules :
                - if anti-cloak scan active, shields take 10% more damage
            */
            auto final_damage = damage;
            auto* sensors = static_cast<ShipSystemSensors*>(m_Ship.getShipSystem(ShipSystemType::Sensors));
            if (sensors) {
                if (sensors->isAntiCloakScanActive()) {
                    final_damage *= 1.1f;
                }
            }
            const float bleed_damage = m_HealthPointsCurrent[shieldSide] - final_damage;
            if (bleed_damage >= 0) {
                apply_damage_amount(source, final_damage, shieldSide);
            }else{
                apply_damage_amount(source, m_HealthPointsCurrent[shieldSide], shieldSide);
                //bleed_damage *= -1.0f;
                //receiveHitBleedDamage(impactNormal, impactModelSpacePosition, impactRadius, maxTime, bleed_damage, shieldSide);
            }
            if (doImpactGraphic) {
                addShieldImpact(impactModelSpacePosition, impactRadius, maxTime);
            }
        }
    }

    /*
    //this logic is really only good for universal (non-sided) shields
    auto& shieldBody = *m_ShieldEntity.getComponent<ComponentBody>();
    if (getHealthCurrent() <= 0) {
        shieldBody.setCollisionMask(CollisionFilter::_Custom_5); //group 5 is misc / blank
    }else{
        shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons
    }
    */
}
void ShipSystemShields::receiveHitBleedDamage(const string& source, const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const float damage, const uint shieldSide) {
    auto* hull = static_cast<ShipSystemHull*>(m_Ship.getShipSystem(ShipSystemType::Hull));
    if (hull) {
        hull->receiveHit(source, impactNormal, impactLocation, impactRadius, damage, 0, false, false); //0 is hard coded model index, but there should not be hull visuals due to bleed damage
    }
}
void ShipSystemShields::apply_damage_amount(const string& source, const float& damage, const uint& shield_side) {
    m_HealthPointsCurrent[shield_side] -= damage;
    //now apply threat

    Map& map = static_cast<Map&>(m_Ship.entity().scene());
    auto& ships = map.getShips();
    if (ships.count(source)) {
        Ship* source_ship = map.getShips().at(source);
        if (source_ship) { //TODO: should not need this nullptr check
            const auto& threat_modifier = Ships::Database[source_ship->getClass()].ThreatModifier;
            auto final_threat_amount = (damage * 10.0f) * threat_modifier; //this 10.0f is just to expand on the unsigned int amount to give more room for modifiers to change the value
            m_Ship.apply_threat(source, static_cast<unsigned int>(final_threat_amount));
        }
    }
}

Entity ShipSystemShields::getEntity() {
    return m_ShieldEntity;
}

void ShipSystemShields::turnOffShields() {
    m_ShieldsAreUp = false;
    auto& shieldBody = *m_ShieldEntity.getComponent<ComponentBody>();
    shieldBody.setCollisionMask(CollisionFilter::_Custom_5); //group 5 is misc / blank
    //TODO: send packet indicating shields are off
}
void ShipSystemShields::turnOnShields() {
    m_ShieldsAreUp = true;
    auto& shieldBody = *m_ShieldEntity.getComponent<ComponentBody>();
    shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons
    //TODO: send packet indicating shields are on
}
const bool ShipSystemShields::shieldsAreUp() const {
    return m_ShieldsAreUp;
}
const float ShipSystemShields::getActualShieldHealthCurrent(const size_t& index) const { return m_HealthPointsCurrent[index]; }
const float ShipSystemShields::getHealthCurrent(const size_t& index) const { return m_ShieldsAreUp ? m_HealthPointsCurrent[index] : 0.0f; }
const float ShipSystemShields::getHealthMax(const size_t& index) const { return m_HealthPointsMax[index]; }
const float ShipSystemShields::getHealthPercent(const size_t& index) const { return getHealthCurrent(index) / getHealthMax(index); }
const float ShipSystemShields::getHealthCurrentForward() const { return m_ShieldsAreUp ? m_HealthPointsCurrent[0] : 0.0f; }
const float ShipSystemShields::getHealthMaxForward() const { return m_HealthPointsMax[0]; }
const float ShipSystemShields::getHealthPercentForward() const { return getHealthCurrentForward() / getHealthMaxForward(); }
const float ShipSystemShields::getHealthCurrentAft() const { return m_ShieldsAreUp ? m_HealthPointsCurrent[1] : 0.0f; }
const float ShipSystemShields::getHealthMaxAft() const { return m_HealthPointsMax[1]; }
const float ShipSystemShields::getHealthPercentAft() const { return getHealthCurrentAft() / getHealthMaxAft(); }
const float ShipSystemShields::getHealthCurrentPort() const { return m_ShieldsAreUp ? m_HealthPointsCurrent[2] : 0.0f; }
const float ShipSystemShields::getHealthMaxPort() const { return m_HealthPointsMax[2]; }
const float ShipSystemShields::getHealthPercentPort() const { return getHealthCurrentPort() / getHealthMaxPort(); }
const float ShipSystemShields::getHealthCurrentStarboard() const { return m_ShieldsAreUp ? m_HealthPointsCurrent[3] : 0.0f; }
const float ShipSystemShields::getHealthMaxStarboard() const { return m_HealthPointsMax[3]; }
const float ShipSystemShields::getHealthPercentStarboard() const { return getHealthCurrentStarboard() / getHealthMaxStarboard(); }
const float ShipSystemShields::getHealthCurrentDorsal() const { return m_ShieldsAreUp ? m_HealthPointsCurrent[4] : 0.0f; }
const float ShipSystemShields::getHealthMaxDorsal() const { return m_HealthPointsMax[4]; }
const float ShipSystemShields::getHealthPercentDorsal() const { return getHealthCurrentDorsal() / getHealthMaxDorsal(); }
const float ShipSystemShields::getHealthCurrentVentral() const { return m_ShieldsAreUp ? m_HealthPointsCurrent[5] : 0.0f; }
const float ShipSystemShields::getHealthMaxVentral() const { return m_HealthPointsMax[5]; }
const float ShipSystemShields::getHealthPercentVentral() const { return getHealthCurrentVentral() / getHealthMaxVentral(); }
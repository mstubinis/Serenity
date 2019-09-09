#include "ShipSystemShields.h"
#include "ShipSystemHull.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ResourceManifest.h"
#include "../../Helper.h"

#include <core/ModelInstance.h>
#include <ecs/Components.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/scene/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

struct ShipSystemShieldsFunctor final { void operator()(ComponentLogic& _component, const double& dt) const {

}};

struct ShieldInstanceBindFunctor {void operator()(EngineResource* r) const {
    Renderer::GLDisable(GL_CULL_FACE);

    auto& i = *static_cast<ModelInstance*>(r);
    auto& shields = *static_cast<ShipSystemShields*>(i.getUserPointer());
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    glm::mat4 parentModel = body.modelMatrix();

    Renderer::sendUniform4Safe("Object_Color", i.color());
    glm::mat4 modelMatrix = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
    int count = 0;
    const auto finalModelPosition = glm::vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        auto& impact = shields.m_ImpactPoints[i];
        if (impact.active) {
            glm::vec3 worldPosition = finalModelPosition + impact.impactLocation;
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
    active = false;
    impactLocation = glm::vec3(0.0f);
    impactRadius = maxTime = currentTime = 0.0f;
    indexInArray = 0;
}
void ShipSystemShieldsImpactPoint::impact(const glm::vec3& _impactLocation, const float& _impactRadius, const float& _maxTime, vector<uint>& freelist) {
    active = true;
    impactLocation = _impactLocation;
    impactRadius = _impactRadius;
    currentTime = 0.0f;
    maxTime = _maxTime;

    removeFromVector(freelist, indexInArray);
}

void ShipSystemShieldsImpactPoint::update(const float& dt, vector<uint>& freelist) {
    if (active) {
        currentTime += dt;
        if (currentTime >= maxTime) {
            active = false;
            currentTime = 0.0f;
            freelist.push_back(indexInArray);
        }
    }
}
#pragma endregion


ShipSystemShields::ShipSystemShields(Ship& _ship, Map& map, const float health) :ShipSystem(ShipSystemType::Shields, _ship){
    m_ShieldEntity = new EntityWrapper(map);
    auto& model = *(m_ShieldEntity->addComponent<ComponentModel>(ResourceManifest::ShieldMesh, ResourceManifest::ShieldMaterial, ResourceManifest::shieldsShaderProgram, RenderStage::ForwardParticles));
    auto& logic = *(m_ShieldEntity->addComponent<ComponentLogic>(ShipSystemShieldsFunctor()));

    //TODO: optimize collision hull if possible
    auto& shieldBody = *(m_ShieldEntity->addComponent<ComponentBody>(CollisionType::TriangleShapeStatic));
    logic.setUserPointer(&m_Ship);
    shieldBody.setUserPointer(this);
    shieldBody.setUserPointer1(&_ship);
    shieldBody.setUserPointer2(&_ship);
    Mesh* shieldColMesh = (Mesh*)ResourceManifest::ShieldColMesh.get();
    Collision* c = new Collision(CollisionType::TriangleShapeStatic, shieldColMesh, shieldBody.mass());
    shieldBody.setCollision(c);

    m_HealthPointsMax = m_HealthPointsCurrent = health;
    //m_TimeSinceLastHit = 10.0f;
    m_RechargeAmount = 450.0f;
    m_RechargeRate = 15.0f;
    //m_RechargeActivation = 5.0f;
    m_RechargeTimer = 0.0f;

    auto& boundBox = m_Ship.getComponent<ComponentModel>()->getModel(0).mesh()->getRadiusBox();
    auto& btBody = const_cast<btRigidBody&>(shieldBody.getBtBody());
    shieldBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    shieldBody.setCollisionGroup(CollisionFilter::_Custom_1); //group 1 are shields
    shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons

    btBody.setUserPointer(&m_Ship);
    shieldBody.setScale((boundBox.x * 1.37f), (boundBox.y * 1.37f), (boundBox.z * 1.37f)); //todo: fix this for when other ships are created


    auto& handles = ResourceManifest::Ships.at(m_Ship.getClass());
    auto& r = handles.get<3>().r;
    auto& g = handles.get<3>().g;
    auto& b = handles.get<3>().b;
    auto& instance = model.getModel(0);
    instance.setColor(r, g, b, 0.7f);
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
    m_ShieldsAreUp = true;
}
ShipSystemShields::~ShipSystemShields() {
}
void ShipSystemShields::destroy() {
    if (m_ShieldEntity) {
        m_ShieldEntity->destroy();
        SAFE_DELETE(m_ShieldEntity);
    }
    m_ShieldsAreUp = false;
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        m_ImpactPoints[i] = ShipSystemShieldsImpactPoint();
    }
}
void ShipSystemShields::update(const double& dt) {
    auto& shieldBody = *m_ShieldEntity->getComponent<ComponentBody>();
    auto& shipBody = *m_Ship.getComponent<ComponentBody>();
    shieldBody.setPosition(shipBody.position());
    shieldBody.setRotation(shipBody.rotation());
    
    bool shown = false;
    const float fdt = static_cast<float>(dt);
    //m_TimeSinceLastHit += fdt;
    for (auto& impact : m_ImpactPoints) {
        impact.update(fdt, m_ImpactPointsFreelist);
        if (!shown && impact.active)
            shown = true;
    }
    auto& shieldModel = m_ShieldEntity->getComponent<ComponentModel>()->getModel();
    !shown ? shieldModel.hide() : shieldModel.show();

    //recharging here
    #pragma region Recharging
    //if (m_TimeSinceLastHit >= m_RechargeActivation) { //only start recharging after not being fired upon for a while
        if (m_HealthPointsCurrent < m_HealthPointsMax) { //dont need to recharge at max shields
            m_RechargeTimer += fdt;
            if (m_RechargeTimer >= m_RechargeRate) {

                m_HealthPointsCurrent += m_RechargeAmount;
                if (shieldBody.getCollisionMask() != CollisionFilter::_Custom_2) {
                    shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons
                }

                if (m_HealthPointsCurrent > m_HealthPointsMax) {
                    m_HealthPointsCurrent = m_HealthPointsMax;
                }
                m_RechargeTimer = 0.0f;
            }
        }
    //}
    #pragma endregion

    ShipSystem::update(dt);
}
void ShipSystemShields::addShieldImpact(const glm::vec3& impactLocationLocal, const float& impactRadius, const float& maxTime) {
    if (m_ImpactPointsFreelist.size() > 0) {
        auto nextIndex = m_ImpactPointsFreelist[0];
        auto& impactPointData = m_ImpactPoints[nextIndex];
        impactPointData.impact(impactLocationLocal, impactRadius, maxTime, m_ImpactPointsFreelist);
    }
}
void ShipSystemShields::receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactLocationLocal, const float& impactRadius, const float& maxTime, const uint damage, const bool doImpactGraphic) {
    if (m_ShieldsAreUp) {
        //m_TimeSinceLastHit = 0.0f;
        const float bleed = m_HealthPointsCurrent - damage;

        if (bleed >= 0) {
            //shields take the entire hit
            m_HealthPointsCurrent -= damage;
        }else{
            m_HealthPointsCurrent = 0.0f;
            //uncomment below to apply bleed damage after shields drop
            /*
            uint bleedDamage = glm::abs(bleed);
            auto* hull = static_cast<ShipSystemHull*>(m_Ship.getShipSystem(ShipSystemType::Hull));
            if (hull) {
                hull->receiveHit(impactNormal, impactLocationLocal, impactRadius, maxTime, bleed);
            }
            */
        }
        if(doImpactGraphic)
            addShieldImpact(impactLocationLocal, impactRadius, maxTime);
    }
    auto& shieldBody = *m_ShieldEntity->getComponent<ComponentBody>();
    if (getHealthCurrent() <= 0) {
        shieldBody.setCollisionMask(CollisionFilter::_Custom_5); //group 5 is misc / blank
    }else{
        shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons
    }
}
EntityWrapper* ShipSystemShields::getEntity() {
    return m_ShieldEntity;
}
const float ShipSystemShields::getHealthCurrent() const {
    return m_ShieldsAreUp ? m_HealthPointsCurrent : 0.0f;
}
const float ShipSystemShields::getHealthMax() const {
    return m_HealthPointsMax;
}
const float ShipSystemShields::getHealthPercent() const {
    return (getHealthCurrent()) / (m_HealthPointsMax);
}
void ShipSystemShields::turnOffShields() {
    m_ShieldsAreUp = false;
    auto& shieldBody = *m_ShieldEntity->getComponent<ComponentBody>();
    shieldBody.setCollisionMask(CollisionFilter::_Custom_5); //group 5 is misc / blank
    //TODO: send packet indicating shields are off
}
void ShipSystemShields::turnOnShields() {
    m_ShieldsAreUp = true;
    auto& shieldBody = *m_ShieldEntity->getComponent<ComponentBody>();
    if (getHealthCurrent() <= 0) {
        shieldBody.setCollisionMask(CollisionFilter::_Custom_5); //group 5 is misc / blank
    }else{
        shieldBody.setCollisionMask(CollisionFilter::_Custom_2); //group 2 are weapons
    }
    //TODO: send packet indicating shields are on
}
const bool ShipSystemShields::shieldsAreUp() const {
    return m_ShieldsAreUp;
}
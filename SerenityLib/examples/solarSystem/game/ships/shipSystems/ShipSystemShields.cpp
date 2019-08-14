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
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());

    Renderer::sendUniform1Safe("AnimationPlaying", 0);

    glm::mat4 modelMatrix = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
    int count = 0;
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        auto& impact = shields.m_ImpactPoints[i];
        if (impact.active) {
            glm::vec3 worldPosition = glm::vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]) + impact.impactLocation;
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

ShipSystemShields::ShipSystemShields(Ship& _ship, Map* map, const uint health) :ShipSystem(ShipSystemType::Shields, _ship), m_ShieldEntity(*map) {
    auto& model = *(m_ShieldEntity.addComponent<ComponentModel>(ResourceManifest::ShieldMesh, ResourceManifest::ShieldMaterial, ResourceManifest::shieldsShaderProgram, RenderStage::ForwardParticles));
    auto& logic = *(m_ShieldEntity.addComponent<ComponentLogic>(ShipSystemShieldsFunctor()));
    auto& body = *(m_ShieldEntity.addComponent<ComponentBody>(CollisionType::Sphere));
    logic.setUserPointer(&m_Ship);
    //logic.setUserPointer1(&model);
    //logic.setUserPointer2(&instance);
    body.setUserPointer(this);
    body.setUserPointer1(&_ship);

    m_HealthPointsMax = m_HealthPointsCurrent = health;
    //m_TimeSinceLastHit = 10.0f;
    m_RechargeAmount = 150;
    m_RechargeRate = 5.0f;
    //m_RechargeActivation = 5.0f;
    m_RechargeTimer = 0.0f;

    auto& boundBox = m_Ship.getComponent<ComponentModel>()->getModel(0).mesh()->getRadiusBox();
    //note: add collision flags -- shields should not collide with other shields or ships. but SHOULD collide with projectiles
    auto& btBody = const_cast<btRigidBody&>(body.getBody());
    body.addCollisionFlag(CollisionFlag::NoContactResponse);

    btBody.setUserPointer(&m_Ship);
    body.setScale((boundBox.x * 1.37f), (boundBox.y * 1.37f), (boundBox.z * 1.37f)); //todo: fix this for when other ships are created


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
void ShipSystemShields::update(const double& dt) {
    auto& shieldBody = *m_ShieldEntity.getComponent<ComponentBody>();
    auto& shipBody = *m_Ship.getComponent<ComponentBody>();
    shieldBody.setPosition(shipBody.position());
    shieldBody.setRotation(shipBody.rotation());
    
    //bool shown = false;
    const float fdt = static_cast<float>(dt);
    //m_TimeSinceLastHit += fdt;
    for (auto& impact : m_ImpactPoints) {
        impact.update(fdt, m_ImpactPointsFreelist);
        //if (!shown && impact.active)
            //shown = true;
    }
    auto& shieldModel = m_ShieldEntity.getComponent<ComponentModel>()->getModel();
    //!shown ? shieldModel.hide() : shieldModel.show();


    //recharging here
    #pragma region Recharging
    //if (m_TimeSinceLastHit >= m_RechargeActivation) { //only start recharging after not being fired upon for a while
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
    //}
    #pragma endregion

    ShipSystem::update(dt);
}
void ShipSystemShields::receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const uint damage) {
    if (m_ShieldsAreUp) {
        //m_TimeSinceLastHit = 0.0f;
        int bleed = m_HealthPointsCurrent - damage;

        if (bleed >= 0) {
            //shields take the entire hit
            m_HealthPointsCurrent -= damage;
        }else{
            //we have damage leaking to the hull
            m_HealthPointsCurrent = 0;
            uint bleedDamage = glm::abs(bleed);
            auto* hull = static_cast<ShipSystemHull*>(m_Ship.getShipSystem(ShipSystemType::Hull));
            if (hull) {
                hull->receiveHit(impactLocation, impactRadius, maxTime, bleed);
            }
        }
        if (m_HealthPointsCurrent > 0 && m_ImpactPointsFreelist.size() > 0) {
            auto nextIndex = m_ImpactPointsFreelist[0];
            auto& impactPointData = m_ImpactPoints[nextIndex];
            impactPointData.impact(impactLocation, impactRadius, maxTime, m_ImpactPointsFreelist);
        }
    }else{
        //TODO: damage the hull entirely, but ideally this should never be reached, keep it at the collision level
        auto* hull = static_cast<ShipSystemHull*>(m_Ship.getShipSystem(ShipSystemType::Hull));
        if (hull) {
            hull->receiveHit(impactLocation, impactRadius, maxTime, damage);
        }
    }
}
const uint ShipSystemShields::getHealthCurrent() const {
    return m_ShieldsAreUp ? m_HealthPointsCurrent : 0;
}
const uint ShipSystemShields::getHealthMax() const {
    return m_HealthPointsMax;
}
const float ShipSystemShields::getHealthPercent() const {
    return static_cast<float>(getHealthCurrent()) / static_cast<float>(m_HealthPointsMax);
}
void ShipSystemShields::turnOffShields() {
    m_ShieldsAreUp = false;
    //TODO: send packet indicating shields are off
}
void ShipSystemShields::turnOnShields() {
    m_ShieldsAreUp = true;
    //TODO: send packet indicating shields are on
}
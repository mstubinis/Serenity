#include "PulsePhaser.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"

#include <ecs/Components.h>
#include <core/engine/math/Engine_Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/lights/Lights.h>

#include <core/engine/Engine.h>
#include <core/engine/materials/Material.h>

#include <ecs/Components.h>
#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>

using namespace Engine;
using namespace std;

struct PulsePhaserCollisionFunctor final { 
void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto pulsePhaserShipVoid    = owner.getUserPointer1();
    auto& pulsePhaserProjectile = *static_cast<PulsePhaserProjectile*>(owner.getUserPointer());

    auto otherPtrShip = other.getUserPointer1(); 
    if (otherPtrShip && pulsePhaserShipVoid) {
        if (otherPtrShip != pulsePhaserShipVoid) {//dont hit ourselves!
            Ship*        otherShip   = static_cast<Ship*>(otherPtrShip);
            if (otherShip && pulsePhaserProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(pulsePhaserShipVoid);
                PulsePhaser& pulsePhaser = *static_cast<PulsePhaser*>(owner.getUserPointer2());
                auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                auto* hull    = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                auto local = otherHit - other.position();
                if (shields && shields->getHealthCurrent() > 0 && other.getUserPointer() == shields) {
                    shields->receiveHit(normal, local, pulsePhaser.impactRadius, pulsePhaser.impactTime, pulsePhaser.damage);
                    pulsePhaserProjectile.destroy();
                    return;
                }
                if (hull && other.getUserPointer() == hull) {
                    hull->receiveHit(normal, local, pulsePhaser.impactRadius, pulsePhaser.impactTime, pulsePhaser.damage);
                    pulsePhaserProjectile.destroy();
                }
            }
        }        
    }
}
};

struct PulsePhaserInstanceBindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    glm::mat4 parentModel = body.modelMatrix();

    glm::mat4 modelMatrix = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PulsePhaserInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    glDepthMask(GL_FALSE);
}};

struct PulsePhaserTailInstanceBindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::vec3 worldPos = glm::vec3(model[3][0], model[3][1], model[3][2]);

    glm::mat4 translation = glm::translate(worldPos);
    glm::mat4 rotationMatrix = glm::mat4_cast(camOrien);
    glm::mat4 scaleMatrix = glm::scale(body.getScale() * i.getScale());

    glm::mat4 modelMatrix = glm::mat4(1.0f) * translation * rotationMatrix * scaleMatrix;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PulsePhaserTailInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_FALSE);
}};


PulsePhaserProjectile::PulsePhaserProjectile(PulsePhaser& source, Map& map, const glm::vec3& position, const glm::vec3& forward) {
    entity = map.createEntity();
    currentTime = 0.0f;
    maxTime = 2.5f;

    auto& model = *entity.addComponent<ComponentModel>(ResourceManifest::CannonEffectMesh, Material::WhiteShadeless,ShaderProgram::Forward,RenderStage::ForwardParticles);
    auto& outline = model.addModel(ResourceManifest::CannonEffectOutlineMesh, ResourceManifest::CannonOutlineMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& head = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& tail = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& cannonBody = *entity.addComponent<ComponentBody>(CollisionType::Box);
    model.setCustomBindFunctor(PulsePhaserInstanceBindFunctor());
    model.setCustomUnbindFunctor(PulsePhaserInstanceUnbindFunctor());
    model.getModel(0).setColor(1.0f, 0.77f, 0.0f, 1.0f);
    outline.setColor(1.0f, 0.43f, 0.0f, 1.0f);
    head.setColor(1.0f, 0.43f, 0.0f, 1.0f);
    tail.setColor(1.0f, 0.43f, 0.0f, 1.0f);

    head.setPosition(0.0f, 0.0f, -0.41996f);
    head.setScale(0.142f, 0.142f, 0.142f);
    tail.setPosition(0.0f, 0.0f, 0.41371f);
    tail.setScale(0.102f, 0.102f, 0.102f);
    head.setCustomBindFunctor(PulsePhaserTailInstanceBindFunctor());
    head.setCustomUnbindFunctor(PulsePhaserTailInstanceUnbindFunctor());
    tail.setCustomBindFunctor(PulsePhaserTailInstanceBindFunctor());
    tail.setCustomUnbindFunctor(PulsePhaserTailInstanceUnbindFunctor());
    
    active = true;
    Ship& s = source.ship;
    auto& shipBody = *s.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position + glm::vec3(0, 0, -model.getModel().mesh()->getRadiusBox().z));
    glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    cannonBody.setPosition(finalPosition);
    cannonBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    cannonBody.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    cannonBody.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();

    cannonBody.setLinearVelocity(shipLinVel, false);
    cannonBody.setAngularVelocity(shipAngVel, false);
    
    auto offset = source.calculatePredictedVector();
    offset *= glm::vec3(source.travelSpeed);
    cannonBody.applyImpulse(offset.x, offset.y, offset.z, false);
    glm::quat q;
    Math::alignTo(q, -offset);
    cannonBody.setRotation(q); //TODO: change rotation based on launching vector

    cannonBody.setUserPointer(this);
    cannonBody.setUserPointer1(&source.ship);
    cannonBody.setUserPointer2(&source);
    cannonBody.setCollisionFunctor(PulsePhaserCollisionFunctor());
    cannonBody.setInternalPhysicsUserPointer(&cannonBody);


    light = new PointLight(finalPosition, &map);
    light->setColor(1.0f, 0.62f, 0.0f, 1.0f);
    light->setAttenuation(LightRange::_20);
}
PulsePhaserProjectile::~PulsePhaserProjectile() {

}
void PulsePhaserProjectile::destroy() {
    if (active) {
        active = false;
        entity.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
    }
}
void PulsePhaserProjectile::update(const double& dt) {
    if (active) {
        const float fdt = static_cast<float>(dt);
        currentTime += fdt;
        if (light) {
            auto& lightBody = *light->getComponent<ComponentBody>();
            lightBody.setPosition(entity.getComponent<ComponentBody>()->position());
        }
        if (currentTime >= maxTime) {
            destroy();
        }
    }
}

PulsePhaser::PulsePhaser(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& _maxCharges, const uint& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume):PrimaryWeaponCannon(ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume), m_Map(map){

}
PulsePhaser::~PulsePhaser() {

}

void PulsePhaser::update(const double& dt) {
    for (auto& projectile : m_ActiveProjectiles) {
        projectile->update(dt);
    }
    for (auto& projectile : m_ActiveProjectiles) {
        if (!projectile->active) {
            removeFromVector(m_ActiveProjectiles, projectile);
        }
    }
    PrimaryWeaponCannon::update(dt);
}
bool PulsePhaser::fire() {
    auto res = PrimaryWeaponCannon::fire();
    if (res) {
        forceFire();
        return true;
    }
    return false;
}
void PulsePhaser::forceFire() {
    auto* projectile = new PulsePhaserProjectile(*this, m_Map, position, forward);
    m_ActiveProjectiles.push_back(projectile);
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);

    auto* sound = Engine::Sound::playEffect(ResourceManifest::SoundPulsePhaser);
    if (sound) {
        sound->setVolume(volume);
        sound->setPosition(finalPosition);
        sound->setAttenuation(0.1f);
    }
}
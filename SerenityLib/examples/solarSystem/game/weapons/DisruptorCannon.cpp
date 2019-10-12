#include "Weapons.h"
#include "../map/Map.h"

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

#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

using namespace Engine;
using namespace std;

struct DisruptorCannonCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto weaponShipVoid = owner.getUserPointer1();
    auto& cannonProjectile = *static_cast<DisruptorCannonProjectile*>(owner.getUserPointer());

    auto otherPtrShip = other.getUserPointer1();
    if (otherPtrShip && weaponShipVoid) {
        if (otherPtrShip != weaponShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherPtrShip);
            if (otherShip && cannonProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(weaponShipVoid);
                if (sourceShip->IsPlayer()) {
                    auto& weapon = *static_cast<DisruptorCannon*>(owner.getUserPointer2());
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    const auto local = otherHit - other.position();

                    if (shields && other.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            cannonProjectile.clientToServerImpact(weapon.m_Map.getClient(), *otherShip, local, normal, weapon.impactRadius, weapon.damage, weapon.impactTime, true);
                            return;
                        }
                    }
                    if (hull && other.getUserPointer() == hull) {
                        cannonProjectile.clientToServerImpact(weapon.m_Map.getClient(), *otherShip, local, normal, weapon.impactRadius, weapon.damage, weapon.impactTime, false);
                    }
                }
            }
        }
    }
}};

struct DisruptorCannonInstanceBindFunctor { void operator()(EngineResource* r) const {
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
struct DisruptorCannonInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_FALSE);
}};

struct DisruptorCannonOutlineInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct DisruptorCannonOutlineInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

struct DisruptorCannonTailInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
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
struct DisruptorCannonTailInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


DisruptorCannonProjectile::DisruptorCannonProjectile(DisruptorCannon& source, Map& map, const glm::vec3& position, const glm::vec3& forward, const int index, const glm::vec3& chosen_target_pos) : PrimaryWeaponCannonProjectile(map, position, forward, index) {
    EntityDataRequest request(entity);

    auto& model = *entity.addComponent<ComponentModel>(request, ResourceManifest::CannonEffectMesh, Material::WhiteShadeless, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& outline = model.addModel(ResourceManifest::CannonEffectOutlineMesh, ResourceManifest::CannonOutlineMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& head = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& tail = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& cannonBody = *entity.addComponent<ComponentBody>(request, CollisionType::Box);
    model.setCustomBindFunctor(DisruptorCannonInstanceBindFunctor());
    model.setCustomUnbindFunctor(DisruptorCannonInstanceUnbindFunctor());
    model.getModel(0).setColor(0.62f, 1.00f, 0.6f, 1.0f);
    outline.setColor(0.31f, 0.9f, 0.28f, 1.0f);
    outline.setCustomBindFunctor(DisruptorCannonOutlineInstanceBindFunctor());
    outline.setCustomUnbindFunctor(DisruptorCannonOutlineInstanceUnbindFunctor());
    head.setColor(0.31f, 0.9f, 0.28f, 1.0f);
    tail.setColor(0.31f, 0.9f, 0.28f, 1.0f);

    head.setPosition(0.0f, 0.0f, -0.38996f);
    head.setScale(0.142f, 0.142f, 0.142f);
    tail.setPosition(0.0f, 0.0f, 0.38371f);
    tail.setScale(0.102f, 0.102f, 0.102f);
    head.setCustomBindFunctor(DisruptorCannonTailInstanceBindFunctor());
    head.setCustomUnbindFunctor(DisruptorCannonTailInstanceUnbindFunctor());
    tail.setCustomBindFunctor(DisruptorCannonTailInstanceBindFunctor());
    tail.setCustomUnbindFunctor(DisruptorCannonTailInstanceUnbindFunctor());

    active = true;
    auto& shipBody = *source.ship.getComponent<ComponentBody>();
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

    auto data = source.calculatePredictedVector(cannonBody, chosen_target_pos);
    auto offset = data.pedictedVector;
    glm::quat q;
    Math::alignTo(q, -offset);
    cannonBody.setRotation(q); //TODO: change rotation based on launching vector
    offset *= glm::vec3(source.travelSpeed);
    cannonBody.applyImpulse(offset.x, offset.y, offset.z, false);

    cannonBody.setUserPointer(this);
    cannonBody.setUserPointer1(&source.ship);
    cannonBody.setUserPointer2(&source);
    cannonBody.setCollisionFunctor(DisruptorCannonCollisionFunctor());
    cannonBody.setInternalPhysicsUserPointer(&cannonBody);
    const_cast<btRigidBody&>(cannonBody.getBtBody()).setDamping(0.0f, 0.0f);


    light = new PointLight(finalPosition, &map);
    light->setColor(0.17f, 0.82f, 0.14f, 1.0f);
    light->setAttenuation(LightRange::_20);
}
DisruptorCannonProjectile::~DisruptorCannonProjectile() {

}

DisruptorCannon::DisruptorCannon(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& _maxCharges, const float& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume) :PrimaryWeaponCannon(map,WeaponType::DisruptorCannon, ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume){

}
DisruptorCannon::~DisruptorCannon() {

}

void DisruptorCannon::update(const double& dt) {
    PrimaryWeaponCannon::update(dt);
}

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
#include <BulletCollision/CollisionShapes/btBoxShape.h>

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
                    const auto local = otherHit - glm::vec3(other.position());

                    if (shields && other.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            cannonProjectile.clientToServerImpact(weapon.m_Map.getClient(), *otherShip, local, normal, weapon.impactRadius, weapon.damage, weapon.impactTime, true);
                            return;
                        }
                    }
                    if (hull && other.getUserPointer() == hull) {
                        /*
                        if (shields) {
                            const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                            if (shields->getHealthCurrent(shieldSide) > 0) {
                                cannonProjectile.clientToServerImpact(weapon.m_Map.getClient(), *otherShip, local, normal, weapon.impactRadius, weapon.damage, weapon.impactTime, true);
                                return;
                            }
                        }
                        */
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
    glm::mat4 scaleMatrix = glm::scale(glm::vec3(body.getScale()) * i.getScale());

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


DisruptorCannonProjectile::DisruptorCannonProjectile(EntityWrapper* target, DisruptorCannon& source, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos) : PrimaryWeaponCannonProjectile(map, final_world_position, forward, index) {
    EntityDataRequest request(entity);

    auto& model = *entity.addComponent<ComponentModel>(request, ResourceManifest::CannonEffectMesh, Material::WhiteShadeless, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& outline = model.addModel(ResourceManifest::CannonEffectOutlineMesh, ResourceManifest::CannonOutlineMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& head = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& tail = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& cannonProjectileBody = *entity.addComponent<ComponentBody>(request, CollisionType::Box);
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

    const auto world_space_dir = Math::rotate_vec3(shipBody.rotation(), forward);
    const auto size = static_cast<decimal>(model.getModel().mesh()->getRadiusBox().z);
    auto finalPosition = final_world_position + (world_space_dir * size);


    auto& sph = *static_cast<btBoxShape*>(cannonProjectileBody.getCollision()->getBtShape());
    sph.setMargin(0.01f);


    cannonProjectileBody.setPosition(finalPosition);
    cannonProjectileBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    cannonProjectileBody.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    cannonProjectileBody.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();

    cannonProjectileBody.setLinearVelocity(shipLinVel, false);
    cannonProjectileBody.setAngularVelocity(shipAngVel, false);

    auto data = source.calculatePredictedVector(target, cannonProjectileBody, chosen_target_pos);
    auto offset = data.pedictedVector;
    glm_quat q;
    Math::alignTo(q, -offset);
    cannonProjectileBody.setRotation(q); //TODO: change rotation based on launching vector
    offset *= glm_vec3(source.travelSpeed);
    cannonProjectileBody.applyImpulse(offset.x, offset.y, offset.z, false);

    cannonProjectileBody.setUserPointer(this);
    cannonProjectileBody.setUserPointer1(&source.ship);
    cannonProjectileBody.setUserPointer2(&source);
    cannonProjectileBody.setCollisionFunctor(DisruptorCannonCollisionFunctor());
    cannonProjectileBody.setInternalPhysicsUserPointer(&cannonProjectileBody);
    const_cast<btRigidBody&>(cannonProjectileBody.getBtBody()).setDamping(0.0f, 0.0f);



    light = new PointLight(finalPosition, &map);
    light->setColor(0.17f, 0.82f, 0.14f, 1.0f);
    light->setAttenuation(LightRange::_20);
}
DisruptorCannonProjectile::~DisruptorCannonProjectile() {

}

DisruptorCannon::DisruptorCannon(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward_vector, const float& arc, const uint& _maxCharges, const float& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const unsigned int& _modelIndex) :PrimaryWeaponCannon(map,WeaponType::DisruptorCannon, ship, position, forward_vector, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _modelIndex){

}
DisruptorCannon::~DisruptorCannon() {

}

void DisruptorCannon::update(const double& dt) {
    PrimaryWeaponCannon::update(dt);
}
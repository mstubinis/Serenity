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
#include <core/engine/physics/Collision.h>

#include <core/engine/Engine.h>
#include <core/engine/materials/Material.h>
#include <core/engine/scene/Camera.h>

#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"
#include <BulletCollision/CollisionShapes/btBoxShape.h>

using namespace Engine;
using namespace std;

struct PulsePhaserCollisionFunctor final { void operator()(CollisionCallbackEventData& data) const {
    auto weaponShipVoid    = data.ownerBody.getUserPointer1();
    auto& cannonProjectile = *static_cast<PulsePhaserProjectile*>(data.ownerBody.getUserPointer());

    auto otherPtrShip = data.otherBody.getUserPointer1(); 
    if (otherPtrShip && weaponShipVoid) {
        if (otherPtrShip != weaponShipVoid) {//dont hit ourselves!
            Ship*        otherShip   = static_cast<Ship*>(otherPtrShip);
            if (otherShip && cannonProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(weaponShipVoid);
                if (sourceShip->IsPlayer()) {
                    auto& weapon = *static_cast<PulsePhaser*>(data.ownerBody.getUserPointer2());
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    auto modelSpacePosition = glm::vec3((glm_vec3(data.otherHit) - data.otherBody.position()) * data.otherBody.rotation());

                    if (shields && data.otherBody.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(modelSpacePosition));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            cannonProjectile.clientToServerImpactShields(true, weapon.m_Map.getClient(), *otherShip, modelSpacePosition, data.normalOnB, weapon.impactRadius, weapon.damage, weapon.impactTime, shieldSide);
                            return;
                        }
                    }
                    if (hull && data.otherBody.getUserPointer() == hull) {
                        /*
                        if (shields) {
                            const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                            if (shields->getHealthCurrent(shieldSide) > 0) {
                                cannonProjectile.clientToServerImpactShields(true, weapon.m_Map.getClient(), *otherShip, modelSpacePosition, data.normalOnB, weapon.impactRadius, weapon.damage, weapon.impactTime, shieldSide);
                                return;
                            }
                        }
                        */
                        cannonProjectile.clientToServerImpactHull(true, weapon.m_Map.getClient(), *otherShip, modelSpacePosition, data.normalOnB, weapon.impactRadius, weapon.damage, weapon.impactTime, data.otherModelInstanceIndex);
                    }
                }
            }
        }        
    }
}};

struct PulsePhaserInstanceBindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    glm::mat4 parentModel = body.modelMatrixRendering();

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

struct PulsePhaserOutlineInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();

    glm::mat4 parentModel = body.modelMatrixRendering();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PulsePhaserOutlineInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

struct PulsePhaserTailInstanceBindFunctor {void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    auto camOrien = cam.getOrientation();

    auto parentModel = body.modelMatrixRendering();
    auto model = parentModel * i.modelMatrix();
    auto worldPos = glm::vec3(model[3][0], model[3][1], model[3][2]);

    auto translation = glm::translate(worldPos);
    auto rotationMatrix = glm::mat4_cast(camOrien);
    auto scaleMatrix = glm::scale(glm::vec3(body.getScale()) * i.getScale());

    auto modelMatrix = glm::mat4(1.0f) * translation * rotationMatrix * scaleMatrix;
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PulsePhaserTailInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


PulsePhaserProjectile::PulsePhaserProjectile(EntityWrapper* target, PulsePhaser& source, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos) : PrimaryWeaponCannonProjectile(map, final_world_position,forward, index){
    EntityDataRequest request(entity);

    auto& model   = *entity.addComponent<ComponentModel>(request, ResourceManifest::CannonEffectMesh, Material::WhiteShadeless,ShaderProgram::Forward,RenderStage::ForwardParticles);
    auto& outline = model.addModel(ResourceManifest::CannonEffectOutlineMesh, ResourceManifest::CannonOutlineMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& head    = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& tail    = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& cannonBody = *entity.addComponent<ComponentBody>(request, CollisionType::Box);
    model.setCustomBindFunctor(PulsePhaserInstanceBindFunctor());
    model.setCustomUnbindFunctor(PulsePhaserInstanceUnbindFunctor());
    model.getModel(0).setColor(1.0f, 0.77f, 0.0f, 1.0f);
    outline.setColor(1.0f, 0.43f, 0.0f, 1.0f);
    outline.setCustomBindFunctor(PulsePhaserOutlineInstanceBindFunctor());
    outline.setCustomUnbindFunctor(PulsePhaserOutlineInstanceUnbindFunctor());
    head.setColor(1.0f, 0.43f, 0.0f, 1.0f);
    tail.setColor(1.0f, 0.43f, 0.0f, 1.0f);

    head.setPosition(0.0f, 0.0f, -0.38996f);
    head.setScale(0.142f, 0.142f, 0.142f);
    tail.setPosition(0.0f, 0.0f, 0.38371f);
    tail.setScale(0.102f, 0.102f, 0.102f);
    head.setCustomBindFunctor(PulsePhaserTailInstanceBindFunctor());
    head.setCustomUnbindFunctor(PulsePhaserTailInstanceUnbindFunctor());
    tail.setCustomBindFunctor(PulsePhaserTailInstanceBindFunctor());
    tail.setCustomUnbindFunctor(PulsePhaserTailInstanceUnbindFunctor());
    
    active = true;
    auto& shipBody = *source.ship.getComponent<ComponentBody>();

    const auto world_space_dir = Math::rotate_vec3(shipBody.rotation(), forward);
    const auto size = static_cast<decimal>(model.getModel().mesh()->getRadiusBox().z);
    auto finalPosition = final_world_position + (world_space_dir * size);

    auto& sph = *static_cast<btBoxShape*>(cannonBody.getCollision()->getBtShape());
    sph.setMargin(0.01f);

    cannonBody.setPosition(finalPosition);
    cannonBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    cannonBody.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    cannonBody.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();

    cannonBody.setLinearVelocity(shipLinVel, false);
    cannonBody.setAngularVelocity(shipAngVel, false);
    
    auto data = source.calculatePredictedVector(target, cannonBody, chosen_target_pos);
    auto offset = data.pedictedVector;
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, -offset);
    cannonBody.setRotation(q); //TODO: change rotation based on launching vector
    offset *= glm_vec3(source.travelSpeed);
    cannonBody.applyImpulse(offset.x, offset.y, offset.z, false);

    cannonBody.setUserPointer(this);
    cannonBody.setUserPointer1(&source.ship);
    cannonBody.setUserPointer2(&source);
    cannonBody.setCollisionFunctor(PulsePhaserCollisionFunctor());
    cannonBody.setInternalPhysicsUserPointer(&cannonBody);
    cannonBody.setDamping(static_cast<decimal>(0.0), static_cast<decimal>(0.0));


    light = new PointLight(finalPosition, &map);
    light->setColor(1.0f, 0.62f, 0.0f, 1.0f);
    light->setAttenuation(LightRange::_32);
}
PulsePhaserProjectile::~PulsePhaserProjectile() {

}

PulsePhaser::PulsePhaser(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward, const float& arc, const uint& _maxCharges, const float& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const unsigned int& _modelIndex):PrimaryWeaponCannon(map,WeaponType::PulsePhaser, ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _modelIndex){

}
PulsePhaser::~PulsePhaser() {

}

void PulsePhaser::update(const double& dt) {
    PrimaryWeaponCannon::update(dt);
}

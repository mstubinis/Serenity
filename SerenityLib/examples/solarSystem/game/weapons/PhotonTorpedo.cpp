#include "PhotonTorpedo.h"
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

struct PhotonTorpedoCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto torpedoShipVoid = owner.getUserPointer1();
    auto& torpedoProjectile = *static_cast<PhotonTorpedoProjectile*>(owner.getUserPointer());

    auto otherPtrShip = other.getUserPointer1();
    if (otherPtrShip && torpedoShipVoid) {
        if (otherPtrShip != torpedoShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherPtrShip);
            if (otherShip && torpedoProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(torpedoShipVoid);
                PhotonTorpedo& torpedo = *static_cast<PhotonTorpedo*>(owner.getUserPointer2());
                auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                auto local = otherHit - other.position();
                if (shields && shields->getHealthCurrent() > 0 && other.getUserPointer() == shields) {
                    shields->receiveHit(normal, local, torpedo.impactRadius, torpedo.impactTime, torpedo.damage);
                    torpedoProjectile.destroy();
                    return;
                }
                if (hull && other.getUserPointer() == hull) {
                    hull->receiveHit(normal, local, torpedo.impactRadius, torpedo.impactTime, torpedo.damage, true);
                    torpedoProjectile.destroy();
                }
            }
        }
    }
}};

struct PhotonTorpedoInstanceCoreBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    const auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    auto worldPos = glm::vec3(model[3][0], model[3][1], model[3][2]);
    modelMatrix = glm::translate(modelMatrix, worldPos);
    modelMatrix *= glm::mat4_cast(camOrien);
    modelMatrix = glm::scale(modelMatrix, body.getScale() * i.getScale());

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PhotonTorpedoInstanceCoreUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


struct PhotonTorpedoInstanceGlowBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    const auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PhotonTorpedoInstanceGlowUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


struct PhotonTorpedoFlareInstanceBindFunctor { void operator()(EngineResource* r) const {
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
struct PhotonTorpedoFlareInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


PhotonTorpedoProjectile::PhotonTorpedoProjectile(PhotonTorpedo& source, Map& map, const glm::vec3& position, const glm::vec3& forward) {
    entityCore = map.createEntity();
    currentTime = 0.0f;
    maxTime = 30.5f;

    auto& model = *entityCore.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoCoreMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& core = model.getModel(0);

    core.setCustomBindFunctor(PhotonTorpedoInstanceCoreBindFunctor());
    core.setCustomUnbindFunctor(PhotonTorpedoInstanceCoreUnbindFunctor());
    core.setColor(1.0f, 1.0f, 1.0f, 1.0f);
   
    auto& torpedoCoreBody = *entityCore.addComponent<ComponentBody>(CollisionType::TriangleShapeStatic);
    torpedoCoreBody.setScale(0.05f);

    entityGlow = map.createEntity();
    auto& model1 = *entityGlow.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& glow = model1.getModel(0);
    glow.setColor(1.0f, 0.43f, 0.0f, 1.0f);
    glow.setCustomBindFunctor(PhotonTorpedoInstanceGlowBindFunctor());
    glow.setCustomUnbindFunctor(PhotonTorpedoInstanceGlowUnbindFunctor());

    active = true;
    Ship& s = source.ship;
    auto& shipBody = *s.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position + glm::vec3(0, 0, -model.getModel().mesh()->getRadiusBox().z));
    glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    torpedoCoreBody.setPosition(finalPosition);
    
    torpedoCoreBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    torpedoCoreBody.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    torpedoCoreBody.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();


    auto offset = source.calculatePredictedVector();
    offset *= glm::vec3(source.travelSpeed);
    

    torpedoCoreBody.setUserPointer(this);
    torpedoCoreBody.setUserPointer1(&source.ship);
    torpedoCoreBody.setUserPointer2(&source);
    torpedoCoreBody.setCollisionFunctor(PhotonTorpedoCollisionFunctor());
    torpedoCoreBody.setInternalPhysicsUserPointer(&torpedoCoreBody);
    torpedoCoreBody.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(torpedoCoreBody.getBtBody()).setDamping(0.0f, 0.0f);


    glm::quat q;
    Math::alignTo(q, -offset);
    torpedoCoreBody.setRotation(q); //TODO: change rotation based on launching vector
    
    light = new PointLight(finalPosition, &map);
    light->setColor(1.0f, 0.62f, 0.0f, 1.0f);
    light->setAttenuation(LightRange::_20);

    auto& torpedoGlowBody = *entityGlow.addComponent<ComponentBody>(CollisionType::None);
    torpedoGlowBody.setPosition(finalPosition);
    torpedoGlowBody.setLinearVelocity(shipLinVel, false);
    torpedoGlowBody.setAngularVelocity(shipAngVel, false);
    torpedoGlowBody.applyImpulse(offset.x, offset.y, offset.z, false);

    torpedoGlowBody.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(torpedoGlowBody.getBtBody()).setDamping(0.0f, 0.0f);
    torpedoGlowBody.setRotation(q);
}
PhotonTorpedoProjectile::~PhotonTorpedoProjectile() {

}
void PhotonTorpedoProjectile::destroy() {
    if (active) {
        active = false;
        entityCore.destroy();
        entityGlow.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
    }
}
void PhotonTorpedoProjectile::update(const double& dt) {
    if (active) {
        const float fdt = static_cast<float>(dt);
        currentTime += fdt;

        auto* activeCam = Resources::getCurrentScene()->getActiveCamera();
        auto& glowBody = *entityGlow.getComponent<ComponentBody>();
        auto glowBodyPos = glowBody.position();
        glowBody.setRotation(activeCam->getOrientation());
        auto& coreBody = *entityCore.getComponent<ComponentBody>();
        coreBody.setPosition(glowBodyPos + glowBody.right());

        if (light) {
            auto& lightBody = *light->getComponent<ComponentBody>();
            lightBody.setPosition(glowBodyPos);
        }
        if (currentTime >= maxTime) {
            destroy();
        }
    }
}

PhotonTorpedo::PhotonTorpedo(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& _maxCharges, const uint& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const float& _rotAngleSpeed) :SecondaryWeaponTorpedo(ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _rotAngleSpeed, false), m_Map(map) {

}
PhotonTorpedo::~PhotonTorpedo() {

}

void PhotonTorpedo::update(const double& dt) {
    for (auto& projectile : m_ActiveProjectiles) {
        projectile->update(dt);
    }
    for (auto& projectile : m_ActiveProjectiles) {
        if (!projectile->active) {
            removeFromVector(m_ActiveProjectiles, projectile);
        }
    }
    SecondaryWeaponTorpedo::update(dt);
}
bool PhotonTorpedo::fire() {
    auto res = SecondaryWeaponTorpedo::fire();
    if (res) {
        forceFire();
        return true;
    }
    return false;
}
void PhotonTorpedo::forceFire() {
    auto* projectile = new PhotonTorpedoProjectile(*this, m_Map, position, forward);
    m_ActiveProjectiles.push_back(projectile);
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);

    auto* sound = Engine::Sound::playEffect(ResourceManifest::SoundPhotonTorpedo);
    if (sound) {
        sound->setVolume(volume);
        sound->setPosition(finalPosition);
        sound->setAttenuation(0.05f);
    }
}
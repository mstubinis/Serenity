#include "PhotonTorpedoOld.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"
#include "../Helper.h"


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

#include <core/engine/renderer/Decal.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>

using namespace Engine;
using namespace std;

struct PhotonTorpedoOldCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto torpedoShipVoid = owner.getUserPointer1();
    auto& torpedoProjectile = *static_cast<PhotonTorpedoOldProjectile*>(owner.getUserPointer());

    auto otherPtrShip = other.getUserPointer1();
    if (otherPtrShip && torpedoShipVoid) {
        if (otherPtrShip != torpedoShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherPtrShip);
            if (otherShip && torpedoProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(torpedoShipVoid);
                PhotonTorpedoOld& torpedo = *static_cast<PhotonTorpedoOld*>(owner.getUserPointer2());
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

struct PhotonTorpedoOldInstanceCoreBindFunctor { void operator()(EngineResource* r) const {
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
struct PhotonTorpedoOldInstanceCoreUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

struct PhotonTorpedoOldInstanceGlowBindFunctor { void operator()(EngineResource* r) const {
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
struct PhotonTorpedoOldInstanceGlowUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


struct PhotonTorpedoOldFlareInstanceBindFunctor { void operator()(EngineResource* r) const {
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
struct PhotonTorpedoOldFlareInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


PhotonTorpedoOldProjectile::PhotonTorpedoOldProjectile(PhotonTorpedoOld& source, Map& map, const glm::vec3& position, const glm::vec3& forward) :torpedo(source) {
    entity = new EntityWrapper(map);
    currentTime = 0.0f;
    maxTime = 30.5f;
    hasLock = false;
    target = nullptr;
    rotationAngleSpeed = source.rotationAngleSpeed;

    EntityDataRequest request(entity->entity());
    EntityDataRequest shipRequest(source.ship.entity());

    auto& model    = *entity->addComponent<ComponentModel>(request, Mesh::Plane, (Material*)(ResourceManifest::TorpedoCoreMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles_2);
    auto& glow     = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& body     = *entity->addComponent<ComponentBody>(request, CollisionType::Sphere);
    
    auto& core = model.getModel(0);
    core.setCustomBindFunctor(PhotonTorpedoOldInstanceCoreBindFunctor());
    core.setCustomUnbindFunctor(PhotonTorpedoOldInstanceCoreUnbindFunctor());
    core.setColor(1.0f, 1.0f, 1.0f, 1.0f);

    btMultiSphereShape& sph = *static_cast<btMultiSphereShape*>(body.getCollision()->getBtShape());
    const auto& _scl = btVector3(0.05f, 0.05f, 0.05f);
    sph.setLocalScaling(_scl);
    sph.setMargin(0.135f);
    sph.setImplicitShapeDimensions(_scl);
    sph.recalcLocalAabb();

    const auto photonRed = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glow.setColor(photonRed);
    glow.setScale(10.6f);
    glow.setCustomBindFunctor(PhotonTorpedoOldInstanceGlowBindFunctor());
    glow.setCustomUnbindFunctor(PhotonTorpedoOldInstanceGlowUnbindFunctor());

    for (uint i = 0; i < 6; ++i) {
        auto& flare = model.addModel(ResourceManifest::TorpedoFlareMesh, ResourceManifest::TorpedoFlareMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
        float randScale = Helper::GetRandomFloatFromTo(-1.1f, 1.1f);

        flare.setScale(2.2f + randScale);
        flare.setColor(photonRed);
        float angle = (360.0f / 6.0f) * i;

        float speed = 0.1f;
        float speedRand = Helper::GetRandomFloatFromTo(0.0f, 1.0f);
        if (speedRand < 0.5f)
            speed *= -1.0f;

        float randX = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
        float randY = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
        float randZ = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
        glm::vec3 randVec = glm::normalize(glm::vec3(randX, randY, randZ));

        glm::vec3 spin = glm::vec3(speed, speed, speed);
        PhotonTorpedoOldFlare flareD = PhotonTorpedoOldFlare(spin);
        flareD.start = glm::angleAxis(glm::radians(angle), randVec);
        flare.setOrientation(flareD.start);

        flares.push_back(flareD);
    }
    active = true;
    glm::vec3 finalPosition = glm::vec3(0.0f);

    auto& shipBody = *source.ship.getComponent<ComponentBody>(shipRequest);
    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    body.setPosition(finalPosition);
    body.setLinearVelocity(shipLinVel, false);
    body.setAngularVelocity(shipAngVel, false);

    body.addCollisionFlag(CollisionFlag::NoContactResponse);
    body.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    body.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    body.setUserPointer(this);
    body.setUserPointer1(&source.ship);
    body.setUserPointer2(&source);
    body.setCollisionFunctor(PhotonTorpedoOldCollisionFunctor());
    body.setInternalPhysicsUserPointer(&body);
    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(body.getBtBody()).setDamping(0.0f, 0.0f);

    auto data = source.calculatePredictedVector(body);
    auto& offset = data.pedictedVector;
    hasLock = data.hasLock;
    target = data.target;
    glm::quat q;
    Math::alignTo(q, -offset);
    body.setRotation(q);
    offset *= glm::vec3(source.travelSpeed);

    body.applyImpulse(offset.x, offset.y, offset.z, false);

    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(body.getBtBody()).setDamping(0.0f, 0.0f);
    body.setRotation(q);

    light = new PointLight(finalPosition, &map);
    light->setColor(photonRed);
    light->setAttenuation(LightRange::_20);
}
PhotonTorpedoOldProjectile::~PhotonTorpedoOldProjectile() {
    entity->destroy();
    SAFE_DELETE(entity);
}
void PhotonTorpedoOldProjectile::destroy() {
    if (active) {
        active = false;
        entity->destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
    }
}
void PhotonTorpedoOldProjectile::update(const double& dt) {
    if (active) {
        const float fdt = static_cast<float>(dt);
        currentTime += fdt;

        auto& glowModel = *entity->getComponent<ComponentModel>();
        auto& body = *entity->getComponent<ComponentBody>();
        auto glowBodyPos = body.position_render();

        //homing logic
        if (hasLock && target) {

        }
        auto* activeCam = glowModel.getOwner().scene().getActiveCamera();
        auto camPos = activeCam->getPosition();

        //TODO: hacky workaround for messed up camera forward vector
        auto vec = glm::normalize(glowBodyPos - camPos);
        vec *= 0.01f;
        body.setRotation(activeCam->getOrientation());
        glowModel.getModel(1).setPosition(vec);

        //flares
        for (uint i = 0; i < flares.size(); ++i) {
            auto& flare = glowModel.getModel(2 + i);
            flare.rotate(flares[i].spin);
        }
        if (light) {
            auto& lightBody = *light->getComponent<ComponentBody>();
            lightBody.setPosition(glowBodyPos);
        }

        if (currentTime >= maxTime) {
            destroy();
        }
    }
}

PhotonTorpedoOld::PhotonTorpedoOld(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& _maxCharges, const float& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const float& _rotAngleSpeed) :SecondaryWeaponTorpedo(ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _rotAngleSpeed), m_Map(map) {

}
PhotonTorpedoOld::~PhotonTorpedoOld() {

}

void PhotonTorpedoOld::update(const double& dt) {
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
const bool PhotonTorpedoOld::fire() {
    const auto res = SecondaryWeaponTorpedo::fire();
    if (res) {
        forceFire();
        return true;
    }
    return false;
}
void PhotonTorpedoOld::forceFire() {
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto* projectile = new PhotonTorpedoOldProjectile(*this, m_Map, position, forward);
    m_ActiveProjectiles.push_back(projectile);
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);

    soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPhotonTorpedoOld);
    if (soundEffect) {
        soundEffect->setVolume(volume);
        soundEffect->setPosition(finalPosition);
        soundEffect->setAttenuation(0.05f);
    }
}
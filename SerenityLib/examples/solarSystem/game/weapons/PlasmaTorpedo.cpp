#include "PlasmaTorpedo.h"
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

#include <ecs/Components.h>
#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>

using namespace Engine;
using namespace std;

struct PlasmaTorpedoCollisionFunctor final {
    void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
        auto torpedoShipVoid = owner.getUserPointer1();
        auto& torpedoProjectile = *static_cast<PlasmaTorpedoProjectile*>(owner.getUserPointer());

        auto otherPtrShip = other.getUserPointer1();
        if (otherPtrShip && torpedoShipVoid) {
            if (otherPtrShip != torpedoShipVoid) {//dont hit ourselves!
                Ship* otherShip = static_cast<Ship*>(otherPtrShip);
                if (otherShip && torpedoProjectile.active) {
                    Ship* sourceShip = static_cast<Ship*>(torpedoShipVoid);
                    PlasmaTorpedo& torpedo = *static_cast<PlasmaTorpedo*>(owner.getUserPointer2());
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
    }
};

struct PlasmaTorpedoInstanceCoreBindFunctor { void operator()(EngineResource* r) const {
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
struct PlasmaTorpedoInstanceCoreUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};
struct PlasmaTorpedoInstanceGlowBindFunctor { void operator()(EngineResource* r) const {
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
struct PlasmaTorpedoInstanceGlowUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};
struct PlasmaTorpedoFlareInstanceBindFunctor { void operator()(EngineResource* r) const {
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
struct PlasmaTorpedoFlareInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


PlasmaTorpedoProjectile::PlasmaTorpedoProjectile(PlasmaTorpedo& source, Map& map, const glm::vec3& position, const glm::vec3& forward) : torpedo(source) {
    entity = new EntityWrapper(map);
    currentTime = 0.0f;
    maxTime = 30.5f;
    hasLock = false;
    target = nullptr;
    rotationAngleSpeed = source.rotationAngleSpeed;

    EntityDataRequest request(entity->entity());
    EntityDataRequest shipRequest(source.ship.entity());

    auto& model = *entity->addComponent<ComponentModel>(request, Mesh::Plane, (Material*)(ResourceManifest::TorpedoCoreMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles_2);
    auto& glow = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlow2Material).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& body = *entity->addComponent<ComponentBody>(request, CollisionType::Sphere);

    auto& core = model.getModel(0);
    core.setCustomBindFunctor(PlasmaTorpedoInstanceCoreBindFunctor());
    core.setCustomUnbindFunctor(PlasmaTorpedoInstanceCoreUnbindFunctor());
    core.setColor(1.0f, 1.0f, 1.0f, 1.0f);

    btMultiSphereShape& sph = *static_cast<btMultiSphereShape*>(body.getCollision()->getBtShape());
    const auto& _scl = btVector3(0.05f, 0.05f, 0.05f);
    sph.setLocalScaling(_scl);
    sph.setMargin(0.135f);
    sph.setImplicitShapeDimensions(_scl);
    sph.recalcLocalAabb();

    const auto plasmaGreen = glm::vec4(0.162f, 0.96f, 0.5f, 1.0f);
    glow.setColor(plasmaGreen);
    glow.setScale(10.6f);
    glow.setCustomBindFunctor(PlasmaTorpedoInstanceGlowBindFunctor());
    glow.setCustomUnbindFunctor(PlasmaTorpedoInstanceGlowUnbindFunctor());

    for (uint i = 0; i < 1; ++i) {
        auto& flare = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::TorpedoFlareTriMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
        flare.setScale(12.2f);
        flare.setColor(plasmaGreen);

        float speed = 0.2f;
        float speedRand = Helper::GetRandomFloatFromTo(0.0f, 1.0f);
        if (speedRand < 0.5f)
            speed *= -1.0f;

        float randAngle = Helper::GetRandomFloatFromTo(0.0f, 360.0f);

        glm::vec3 spin = glm::vec3(0, 0, speed);
        PlasmaTorpedoFlare flareD = PlasmaTorpedoFlare(spin);
        flareD.start = glm::angleAxis(glm::radians(randAngle), glm::vec3(0, 0, 1));
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
    body.setCollisionFunctor(PlasmaTorpedoCollisionFunctor());
    body.setInternalPhysicsUserPointer(&body);
    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(body.getBtBody()).setDamping(0.0f, 0.0f);

    light = new PointLight(finalPosition, &map);
    light->setColor(plasmaGreen);
    light->setAttenuation(LightRange::_20);

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
}
PlasmaTorpedoProjectile::~PlasmaTorpedoProjectile() {
    entity->destroy();
    SAFE_DELETE(entity);
}
void PlasmaTorpedoProjectile::destroy() {
    if (active) {
        active = false;
        entity->destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
    }
}
void PlasmaTorpedoProjectile::update(const double& dt) {
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

PlasmaTorpedo::PlasmaTorpedo(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& _maxCharges, const uint& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const float& _rotAngleSpeed) :SecondaryWeaponTorpedo(ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _rotAngleSpeed), m_Map(map) {

}
PlasmaTorpedo::~PlasmaTorpedo() {

}

void PlasmaTorpedo::update(const double& dt) {
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
const bool PlasmaTorpedo::fire() {
    auto res = SecondaryWeaponTorpedo::fire();
    if (res) {
        forceFire();
        return true;
    }
    return false;
}
void PlasmaTorpedo::forceFire() {
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto* projectile = new PlasmaTorpedoProjectile(*this, m_Map, position, forward);
    m_ActiveProjectiles.push_back(projectile);
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);

    auto* sound = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaTorpedo);
    if (sound) {
        sound->setVolume(volume);
        sound->setPosition(finalPosition);
        sound->setAttenuation(0.05f);
    }
}
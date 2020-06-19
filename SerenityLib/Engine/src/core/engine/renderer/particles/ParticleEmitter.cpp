#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/system/Engine.h>
#include <core/engine/resources/Engine_Resources.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

#include <mutex>

using namespace std;
using namespace Engine;

ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) : Entity(scene){
    addComponent<ComponentBody>();

    /*
    addComponent<ComponentModel>(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), Material::Checkers);
    auto& modelComponent = *getComponent<ComponentModel>();
    modelComponent.getModel().setScale(0.01f, 0.01f, 0.1f);
    modelComponent.getModel().translate(0.0f, 0.0f, 0.1f);
    */
    init(properties, scene, lifetime, parent);
}
void ParticleEmitter::init(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    setProperties(properties);
    m_Parent        = parent;
    m_Lifetime      = lifetime;
    m_SpawningTimer = m_Properties->m_SpawnRate - 0.01f;

    setLinearVelocity(0, 0, 0);
    setScale(1, 1, 1);
}

ParticleEmitter::~ParticleEmitter() {

}

ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept {
    m_Properties      = std::exchange(other.m_Properties, nullptr);
    m_SpawningTimer   = std::move(other.m_SpawningTimer);
    m_Active          = std::move(other.m_Active);
    m_Timer           = std::move(other.m_Timer);
    m_Lifetime        = std::move(other.m_Lifetime);
    m_Parent          = std::move(other.m_Parent);
    m_Data            = std::move(other.m_Data);
    m_UpdateFunctor   = std::move(other.m_UpdateFunctor);
    m_UserData        = std::move(other.m_UserData);
}
ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& other) noexcept {     
    m_Properties      = std::exchange(other.m_Properties, nullptr);
    m_SpawningTimer   = std::move(other.m_SpawningTimer);
    m_Active          = std::move(other.m_Active);
    m_Timer           = std::move(other.m_Timer);
    m_Lifetime        = std::move(other.m_Lifetime);
    m_Parent          = std::move(other.m_Parent);
    m_Data            = std::move(other.m_Data);
    m_UpdateFunctor   = std::move(other.m_UpdateFunctor);
    m_UserData        = std::move(other.m_UserData); 
    return *this;
}

void ParticleEmitter::activate() {
    m_Active = true;
}
void ParticleEmitter::deactivate() {
    m_Active = false;
}

ParticleEmissionProperties* ParticleEmitter::getProperties() const {
    return m_Properties;
}
void ParticleEmitter::setProperties(ParticleEmissionProperties& properties) {
    m_Properties = &properties;
}


void ParticleEmitter::update(size_t index, const float dt, priv::ParticleSystem& particleSystem, bool multi_threaded) {
    //handle spawning
    if (m_Active) {
        m_Timer           += dt;
        m_SpawningTimer   += dt;
        auto& properties   = *m_Properties;
        m_UpdateFunctor(this, dt, std::ref(properties), std::ref(particleSystem.m_Mutex));
        if (m_SpawningTimer > properties.m_SpawnRate) {
            {
                if (multi_threaded) {
                    std::lock_guard lock(particleSystem.m_Mutex);
                    for (unsigned int i = 0; i < properties.m_ParticlesPerSpawn; ++i) {
                        particleSystem.add_particle(*this);
                    }
                }else{
                    for (unsigned int i = 0; i < properties.m_ParticlesPerSpawn; ++i) {
                        particleSystem.add_particle(*this);
                    }
                }
            }
            m_SpawningTimer = 0.0;
        }
        if (m_Lifetime > 0.0 && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer  = 0.0;

            if (multi_threaded) {
                std::lock_guard lock(particleSystem.m_Mutex);
                particleSystem.m_ParticleEmitterFreelist.push(index);
            }else{
                particleSystem.m_ParticleEmitterFreelist.push(index);
            }
        }
    }
}
bool ParticleEmitter::isActive() const {
    return m_Active;
}

void ParticleEmitter::setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w) {
    getComponent<ComponentBody>()->setRotation(x, y, z, w);
}
void ParticleEmitter::setRotation(const glm_quat& rotation) {
    getComponent<ComponentBody>()->setRotation(rotation);
}

void ParticleEmitter::rotate(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->rotate(x, y, z);
}

void ParticleEmitter::setPosition(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setPosition(x, y, z);
}
void ParticleEmitter::setPosition(const glm_vec3& position) {
    getComponent<ComponentBody>()->setPosition(position);
}


void ParticleEmitter::setScale(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setScale(x, y, z);
}
void ParticleEmitter::setScale(const glm_vec3& scale) {
    getComponent<ComponentBody>()->setScale(scale);
}

glm_vec3 ParticleEmitter::getScale() const {
    return getComponent<ComponentBody>()->getScale();
}
glm_vec3 ParticleEmitter::position() const {
    return getComponent<ComponentBody>()->getPosition();
}

glm_quat ParticleEmitter::rotation() const {
    return getComponent<ComponentBody>()->getRotation();
}

void ParticleEmitter::setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->setLinearVelocity(x, y, z, local);
}
void ParticleEmitter::setLinearVelocity(const glm_vec3& lv, const bool local) {
    getComponent<ComponentBody>()->setLinearVelocity(lv, local);
}

void ParticleEmitter::applyLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    auto& body = *getComponent<ComponentBody>();
    const auto currVel = body.getLinearVelocity();
    auto newVel = glm_vec3(x, y, z);
    if (local) {
        newVel = body.getRotation() * newVel;
    }
    body.setLinearVelocity(currVel + newVel, false);
}
void ParticleEmitter::applyLinearVelocity(glm_vec3& velocity, const bool local) {
    auto& body = *getComponent<ComponentBody>();
    const auto currVel = body.getLinearVelocity();
    if (local) {
        velocity = body.getRotation() * velocity;
    }
    body.setLinearVelocity(currVel + velocity, false);
}

glm_vec3 ParticleEmitter::linearVelocity() const {
    return getComponent<ComponentBody>()->getLinearVelocity();
}
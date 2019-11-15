#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/resources/Engine_Resources.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

#include <mutex>

using namespace std;
using namespace Engine;

ParticleEmitter::ParticleEmitter() : EntityWrapper(*Resources::getCurrentScene()) {
    m_Properties = nullptr;
    m_Active = false;
    m_Parent = Entity::_null;
    m_Lifetime = 0.0;
    internal_init();

}
ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime, EntityWrapper* parent) : ParticleEmitter(properties, scene, lifetime, (parent) ? parent->entity() : Entity::_null){}
ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime, Entity& parent) : EntityWrapper(scene){
    setProperties(properties);
    m_Active = true;
    m_Parent = parent;
    m_Lifetime = lifetime;
    internal_init();
}
void ParticleEmitter::internal_init() {
    m_SpawningTimer = m_Properties->m_SpawnRate - 0.01f;
    m_Timer = 0.0;
    addComponent<ComponentBody>();

    //auto& modelComponent = *addComponent<ComponentModel>(Mesh::Cube, Material::Checkers);
    //modelComponent.getModel().setScale(0.1f, 0.1f, 0.1f);
}
ParticleEmitter::~ParticleEmitter() {

}


ParticleEmitter::ParticleEmitter(const ParticleEmitter& other) : EntityWrapper(other) {
    m_Properties = other.m_Properties;
    m_SpawningTimer = other.m_SpawningTimer;
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Lifetime = other.m_Lifetime;
    m_Parent = other.m_Parent;
}
ParticleEmitter& ParticleEmitter::operator=(const ParticleEmitter& other) {
    if (&other == this)
        return *this;
    m_Properties = other.m_Properties;
    m_SpawningTimer = other.m_SpawningTimer;
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Lifetime = other.m_Lifetime;
    m_Parent = other.m_Parent;
    return *this;
}
ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept : EntityWrapper(other) {
    using std::swap;
    swap(m_Properties, other.m_Properties);
    swap(m_SpawningTimer, other.m_SpawningTimer);
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Lifetime, other.m_Lifetime);
    swap(m_Parent, other.m_Parent);
}
ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& other) noexcept {
    using std::swap;
    swap(m_Properties, other.m_Properties);
    swap(m_SpawningTimer, other.m_SpawningTimer);
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Lifetime, other.m_Lifetime);
    swap(m_Parent, other.m_Parent);
    return *this;
}

void ParticleEmitter::activate() {
    m_Active = true;
}
void ParticleEmitter::deactivate() {
    m_Active = false;
}


void ParticleEmitter::setProperties(ParticleEmissionProperties& properties) {
    m_Properties = &properties;
}


void ParticleEmitter::update_multithreaded(const size_t& index, const double& dt, epriv::ParticleSystem& particleSystem) {
    //handle spawning
    if (m_Active) {
        m_Timer += dt;
        m_SpawningTimer += dt;
        auto& properties = *m_Properties;
        if (m_SpawningTimer > properties.m_SpawnRate) {
            particleSystem.m_Mutex.lock();
            for(unsigned int i = 0; i < properties.m_ParticlesPerSpawn; ++i)
                particleSystem.add_particle(*this);
            particleSystem.m_Mutex.unlock();
            m_SpawningTimer = 0.0;
        }
        if (m_Lifetime > 0.0 && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer = 0.0;
            particleSystem.m_Mutex.lock();
            particleSystem.m_ParticleEmitterFreelist.push(index);
            particleSystem.m_Mutex.unlock();
        }
    }
}
void ParticleEmitter::update(const size_t& index, const double& dt, epriv::ParticleSystem& particleSystem) {
    //handle spawning
    if (m_Active) {
        m_Timer += dt;
        m_SpawningTimer += dt;
        auto& properties = *m_Properties;
        if (m_SpawningTimer > properties.m_SpawnRate) {
            for (unsigned int i = 0; i < properties.m_ParticlesPerSpawn; ++i)
                particleSystem.add_particle(*this);
            m_SpawningTimer = 0.0;
        }
        if (m_Lifetime > 0.0 && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer = 0.0;
            particleSystem.m_ParticleEmitterFreelist.push(index);
        }
    }
}

const bool& ParticleEmitter::isActive() const {
    return m_Active;
}


void ParticleEmitter::setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w, EntityDataRequest& request) {
    getComponent<ComponentBody>(request)->setRotation(x, y, z, w);
}
void ParticleEmitter::setRotation(const glm_quat& rotation, EntityDataRequest& request) {
    getComponent<ComponentBody>(request)->setRotation(rotation);
}
void ParticleEmitter::setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w) {
    getComponent<ComponentBody>()->setRotation(x, y, z, w);
}
void ParticleEmitter::setRotation(const glm_quat& rotation) {
    getComponent<ComponentBody>()->setRotation(rotation);
}


void ParticleEmitter::setPosition(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request) {
    getComponent<ComponentBody>(request)->setPosition(x, y, z);
}
void ParticleEmitter::setPosition(const glm_vec3& position, EntityDataRequest& request) {
    getComponent<ComponentBody>(request)->setPosition(position);
}
void ParticleEmitter::setPosition(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setPosition(x, y, z);
}
void ParticleEmitter::setPosition(const glm_vec3& position) {
    getComponent<ComponentBody>()->setPosition(position);
}


void ParticleEmitter::setScale(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request) {
    getComponent<ComponentBody>(request)->setScale(x, y, z);
}
void ParticleEmitter::setScale(const glm_vec3& scale, EntityDataRequest& request) {
    getComponent<ComponentBody>(request)->setScale(scale);
}
void ParticleEmitter::setScale(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setScale(x, y, z);
}
void ParticleEmitter::setScale(const glm_vec3& scale) {
    getComponent<ComponentBody>()->setScale(scale);
}

const glm_vec3 ParticleEmitter::getScale(EntityDataRequest& request) {
    return getComponent<ComponentBody>(request)->getScale();
}
const glm_vec3 ParticleEmitter::getScale() {
    return getComponent<ComponentBody>()->getScale();
}
const glm_vec3 ParticleEmitter::position(EntityDataRequest& request) {
    return getComponent<ComponentBody>(request)->position();
}
const glm_vec3 ParticleEmitter::position() {
    return getComponent<ComponentBody>()->position();
}

const glm_quat ParticleEmitter::rotation(EntityDataRequest& request) {
    return getComponent<ComponentBody>(request)->rotation();
}
const glm_quat ParticleEmitter::rotation() {
    return getComponent<ComponentBody>()->rotation();
}
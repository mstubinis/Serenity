#include <core/engine/renderer/ParticleEmitter.h>
#include <core/engine/renderer/ParticleEmissionProperties.h>
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
    m_Lifetime = 0.0;
    internal_init();

}
ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime) : EntityWrapper(scene){
    setProperties(properties);
    m_Active = true;
    m_Lifetime = lifetime;
    internal_init();
}
void ParticleEmitter::internal_init() {
    m_SpawningTimer = 0.0;
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
}
ParticleEmitter& ParticleEmitter::operator=(const ParticleEmitter& other) {
    if (&other == this)
        return *this;
    m_Properties = other.m_Properties;
    m_SpawningTimer = other.m_SpawningTimer;
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Lifetime = other.m_Lifetime;
    return *this;
}
ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept : EntityWrapper(other) {
    using std::swap;
    swap(m_Properties, other.m_Properties);
    swap(m_SpawningTimer, other.m_SpawningTimer);
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Lifetime, other.m_Lifetime);
}
ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& other) noexcept {
    using std::swap;
    swap(m_Properties, other.m_Properties);
    swap(m_SpawningTimer, other.m_SpawningTimer);
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Lifetime, other.m_Lifetime);
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


void ParticleEmitter::update_multithreaded(const unsigned int& index, const double& dt, mutex& mutex_, std::vector<Particle>& particles, stack<unsigned int>& particle_freelist, stack<unsigned int>& emitter_freelist) {
    //handle spawning
    if (m_Active) {
        m_Timer += dt;
        m_SpawningTimer += dt;
        auto& properties = *m_Properties;
        if (m_SpawningTimer > properties.m_SpawnRate) {
            mutex_.lock();
            auto& bodyComponent = *getComponent<ComponentBody>();
            spawnParticle(particles, particle_freelist, bodyComponent.position(), bodyComponent.rotation());
            mutex_.unlock();
            m_SpawningTimer = 0.0;
        }
        if (m_Lifetime > 0.0 && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer = 0.0;
            emitter_freelist.push(index);
        }
    }
}
void ParticleEmitter::update(const unsigned int& index, const double& dt, std::vector<Particle>& particles, stack<unsigned int>& particle_freelist, stack<unsigned int>& emitter_freelist) {
    //handle spawning
    if (m_Active) {
        m_Timer += dt;
        m_SpawningTimer += dt;
        auto& properties = *m_Properties;
        if (m_SpawningTimer > properties.m_SpawnRate) {
            auto& bodyComponent = *getComponent<ComponentBody>();
            spawnParticle(particles, particle_freelist, bodyComponent.position(), bodyComponent.rotation());
            m_SpawningTimer = 0.0;
        }
        if (m_Lifetime > 0.0 && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer = 0.0;
            emitter_freelist.push(index);
        }
    }
}
const bool ParticleEmitter::spawnParticle(vector<Particle>& particles, stack<unsigned int>& freelist, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {

    //first, try to reuse an empty
    if (freelist.size() > 0) {
        ParticleData data(*m_Properties);
        auto freeindex = freelist.top();
        particles[freeindex].init(data, emitterPosition, emitterRotation);
        freelist.pop();
        return true;
    }
    //otherwise...

    if (particles.size() < particles.capacity()) {
        Particle particle(emitterPosition, emitterRotation , *m_Properties, entity().scene());
        particles.push_back(std::move(particle));
        return true;
    }
    return false;
}
const bool ParticleEmitter::spawnParticle(vector<Particle>& particles, stack<unsigned int>& freelist) {
    auto& body = *getComponent<ComponentBody>();
    return ParticleEmitter::spawnParticle(particles, freelist, body.position(), body.rotation());
}

const bool& ParticleEmitter::isActive() const {
    return m_Active;
}
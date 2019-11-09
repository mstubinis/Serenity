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
    m_Particles.reserve(0);
    m_LastIndex = 0;
    m_Properties = nullptr;
    m_Active = false;
    internal_init();
}
ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const size_t maxParticles) : EntityWrapper(scene){
    m_Particles.reserve(maxParticles);
    m_LastIndex = maxParticles;
    setProperties(properties);
    m_Active = true;
    internal_init();
}
void ParticleEmitter::internal_init() {
    m_SpawningTimer = 0.0;
    addComponent<ComponentBody>();

    //auto& modelComponent = *addComponent<ComponentModel>(Mesh::Cube, Material::Checkers);
    //modelComponent.getModel().setScale(0.1f, 0.1f, 0.1f);
}
ParticleEmitter::~ParticleEmitter() {

}


ParticleEmitter::ParticleEmitter(const ParticleEmitter& other) : EntityWrapper(other) {
    m_LastIndex = other.m_LastIndex;
    m_Properties = other.m_Properties;
    m_SpawningTimer = other.m_SpawningTimer;
    m_Active = other.m_Active;
    m_Particles = other.m_Particles;
}
ParticleEmitter& ParticleEmitter::operator=(const ParticleEmitter& other) {
    if (&other == this)
        return *this;
    m_LastIndex = other.m_LastIndex;
    m_Properties = other.m_Properties;
    m_SpawningTimer = other.m_SpawningTimer;
    m_Active = other.m_Active;
    m_Particles = other.m_Particles;
    return *this;
}
ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept : EntityWrapper(other) {
    using std::swap;
    swap(m_LastIndex, other.m_LastIndex);
    swap(m_Properties, other.m_Properties);
    swap(m_SpawningTimer, other.m_SpawningTimer);
    swap(m_Active, other.m_Active);
    swap(m_Particles, other.m_Particles);
}
ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& other) noexcept {
    using std::swap;
    swap(m_LastIndex, other.m_LastIndex);
    swap(m_Properties, other.m_Properties);
    swap(m_SpawningTimer, other.m_SpawningTimer);
    swap(m_Active, other.m_Active);
    swap(m_Particles, other.m_Particles);
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
void ParticleEmitter::internal_particles_update(const double& dt) {
    bool marked_index = false;
    for (size_t i = 0; i < m_Particles.size(); ++i) {
        auto& particle = m_Particles[i];
        particle.update(dt);
        if (!particle.isActive() && !marked_index) {
            m_LastIndex = i;
            marked_index = true;
        }
    }
}
void ParticleEmitter::internal_particles_update_multithreaded(const double& dt, mutex& mutex_) {
    bool marked_index = false;
    for (size_t i = 0; i < m_Particles.size(); ++i) {
        auto& particle = m_Particles[i];
        particle.update_multithreaded(dt, mutex_);
        if (!particle.isActive() && !marked_index) {
            m_LastIndex = i;
            marked_index = true;
        }
    }
}
void ParticleEmitter::update_multithreaded(const double& dt, mutex& mutex_) {
    //handle spawning
    if (m_Active) {
        m_SpawningTimer += dt;
        auto& properties = *m_Properties;
        if (m_SpawningTimer > properties.m_SpawnRate) {
            mutex_.lock();
            auto& bodyComponent = *getComponent<ComponentBody>();
            spawnParticle(bodyComponent.position(), bodyComponent.rotation(), properties.m_Stage);
            mutex_.unlock();
            m_SpawningTimer = 0.0;
        }
    }
    internal_particles_update_multithreaded(dt, mutex_);
}
void ParticleEmitter::update(const double& dt) {
    //handle spawning
    if (m_Active) {
        m_SpawningTimer += dt;
        auto& properties = *m_Properties;
        if (m_SpawningTimer > properties.m_SpawnRate) {
            auto& bodyComponent = *getComponent<ComponentBody>();
            spawnParticle(bodyComponent.position(), bodyComponent.rotation(), properties.m_Stage);
            m_SpawningTimer = 0.0;
        }
    }
    internal_particles_update(dt);
}
const bool ParticleEmitter::spawnParticle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, const RenderStage::Stage stage) {
    if (m_Particles.size() < m_Particles.capacity()) {
        Particle particle(emitterPosition, emitterRotation , *m_Properties, entity().scene(), stage);
        m_Particles.push_back(std::move(particle));
        return true;
    }else{
        //reuse the first dead particle
        if (m_LastIndex < m_Particles.size()) {
            ParticleData data(*m_Properties);
            m_Particles[m_LastIndex].init(data, emitterPosition, emitterRotation, *m_Particles[m_LastIndex].getComponent<ComponentBody>(), *m_Particles[m_LastIndex].getComponent<ComponentModel>());
            return true;
        }
    }
    return false;
}
const bool ParticleEmitter::spawnParticle(const RenderStage::Stage stage) {
    auto& body = *getComponent<ComponentBody>();
    return ParticleEmitter::spawnParticle(body.position(), body.rotation(), stage);
}

const bool& ParticleEmitter::isActive() const {
    return m_Active;
}
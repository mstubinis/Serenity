#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>

using namespace std;
using namespace Engine;


struct DefaultColorFunctor final { Engine::color_vector_4 operator()(ParticleEmissionProperties& properties, const float particle_lifetime, const float dt, ParticleEmitter* emitter, Particle& particle) const {
    return Engine::color_vector_4(255_uc);
} };
struct DefaultAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const float particle_lifetime, const float dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct DefaultVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const float particle_lifetime, const float dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec3(0.0f);
}};
struct DefaultScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const float particle_lifetime, const float dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec2(0.0f);
}};
struct DefaultDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const float particle_lifetime, const float dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};


struct DefaultInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f); //0.0f to 1.0f
    const auto random_amount_y = (static_cast<float>((rand() % 101)     ) / 100.0f); //0.0f to 1.0f
    const auto random_amount_z = (static_cast<float>((rand() % 101) - 50) / 50.0f); //0.0f to 1.0f
    return glm::vec3(random_amount_x * 0.008f, random_amount_y * 0.05f, random_amount_z * 0.008f);
}};
struct DefaultInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle) const {
    return glm::vec2(0.3f);
}};
struct DefaultInitialAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle) const {
    return 0.0f;
}};



ParticleEmissionProperties ParticleEmissionProperties::DefaultProperties;

ParticleEmissionProperties::ParticleEmissionProperties(){
    m_Lifetime = 4.0;
    m_SpawnRate = 0.4f;

    setColorFunctor(DefaultColorFunctor());
    setChangeInAngularVelocityFunctor(DefaultAngularVelocityFunctor());
    setChangeInVelocityFunctor(DefaultVelocityFunctor());
    setChangeInScaleFunctor(DefaultScaleFunctor());
    setDepthFunctor(DefaultDepthFunctor());

    setInitialVelocityFunctor(DefaultInitialVelocityFunctor());
    setInitialScaleFunctor(DefaultInitialScaleFunctor());
    setInitialAngularVelocityFunctor(DefaultInitialAngularVelocityFunctor());
}

ParticleEmissionProperties::ParticleEmissionProperties(Handle& materialHandle, const float lifeTime, const float spawnRate, const unsigned int ParticlesPerSpawn, const float drag) : ParticleEmissionProperties(){
    m_Lifetime = lifeTime;
    m_Drag = drag;
    m_SpawnRate = spawnRate;
    m_ParticleMaterials.push_back(Resources::getMaterial(materialHandle));
    m_ParticlesPerSpawn = ParticlesPerSpawn;
}
ParticleEmissionProperties::~ParticleEmissionProperties() {
    m_ParticleMaterials.clear();
}

ParticleEmissionProperties::ParticleEmissionProperties(ParticleEmissionProperties&& other) noexcept {
    m_Lifetime          = std::move(other.m_Lifetime);
    m_Drag              = std::move(other.m_Drag);
    m_SpawnRate         = std::move(other.m_SpawnRate);
    m_ParticlesPerSpawn = std::move(other.m_ParticlesPerSpawn);
    m_ParticleMaterials = std::move(other.m_ParticleMaterials);
    m_ColorFunctor.swap(other.m_ColorFunctor);
    m_ChangeInAngularVelocityFunctor.swap(other.m_ChangeInAngularVelocityFunctor);
    m_ChangeInVelocityFunctor.swap(other.m_ChangeInVelocityFunctor);
    m_ChangeInScaleFunctor.swap(other.m_ChangeInScaleFunctor);
    m_DepthFunctor.swap(other.m_DepthFunctor);
    m_InitialVelocityFunctor.swap(other.m_InitialVelocityFunctor);
    m_InitialScaleFunctor.swap(other.m_InitialScaleFunctor);
    m_InitialAngularVelocityFunctor.swap(other.m_InitialAngularVelocityFunctor);   
}
ParticleEmissionProperties& ParticleEmissionProperties::operator=(ParticleEmissionProperties&& other) noexcept {
    if (&other != this) {
        m_Lifetime          = std::move(other.m_Lifetime);
        m_Drag              = std::move(other.m_Drag);
        m_SpawnRate         = std::move(other.m_SpawnRate);
        m_ParticlesPerSpawn = std::move(other.m_ParticlesPerSpawn);
        m_ParticleMaterials = std::move(other.m_ParticleMaterials);
        m_ColorFunctor.swap(other.m_ColorFunctor);
        m_ChangeInAngularVelocityFunctor.swap(other.m_ChangeInAngularVelocityFunctor);
        m_ChangeInVelocityFunctor.swap(other.m_ChangeInVelocityFunctor);
        m_ChangeInScaleFunctor.swap(other.m_ChangeInScaleFunctor);
        m_DepthFunctor.swap(other.m_DepthFunctor);
        m_InitialVelocityFunctor.swap(other.m_InitialVelocityFunctor);
        m_InitialScaleFunctor.swap(other.m_InitialScaleFunctor);
        m_InitialAngularVelocityFunctor.swap(other.m_InitialAngularVelocityFunctor);
    }
    return *this;
}

const float ParticleEmissionProperties::getLifetime() const {
    return m_Lifetime;
}
const float ParticleEmissionProperties::getSpawnRate() const {
    return m_SpawnRate;
}

const bool ParticleEmissionProperties::addMaterial(Material& material) {
    for (auto& mat : m_ParticleMaterials) {
        if (&material == mat) {
            return false;
        }
    }
    m_ParticleMaterials.push_back(&material);
    return true;
}
const bool ParticleEmissionProperties::addMaterial(Handle& materialHandle) {
    return ParticleEmissionProperties::addMaterial(*Resources::getMaterial(materialHandle));
}

const Material& ParticleEmissionProperties::getParticleMaterialRandom() const {
    if (m_ParticleMaterials.size() == 0) {
        return *Material::Checkers;
    }
    const auto index = rand() % m_ParticleMaterials.size();
    return *m_ParticleMaterials[index];
}
const Material& ParticleEmissionProperties::getParticleMaterial(const size_t index) const {
    if (m_ParticleMaterials.size() == 0) {
        return *Material::Checkers;
    }
    return *m_ParticleMaterials[index];
}
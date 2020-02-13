#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>

using namespace std;
using namespace Engine;


struct DefaultColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const float& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec4(1.0f);
} };
struct DefaultAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const float& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct DefaultVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const float& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec3(0.0f);
}};
struct DefaultScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const float& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec2(0.0f);
}};
struct DefaultDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const float& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};


struct DefaultInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f); //0.0f to 1.0f
    const auto random_amount_y = (static_cast<float>((rand() % 101)     ) / 100.0f); //0.0f to 1.0f
    const auto random_amount_z = (static_cast<float>((rand() % 101) - 50) / 50.0f); //0.0f to 1.0f
    return glm::vec3(random_amount_x * 0.008f, random_amount_y * 0.05f, random_amount_z * 0.008f);
}};
struct DefaultInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.3f);
}};
struct DefaultInitialAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return 0.0f;
}};



ParticleEmissionProperties ParticleEmissionProperties::DefaultProperties;

ParticleEmissionProperties::ParticleEmissionProperties(){
    m_Lifetime = 4.0;
    m_SpawnRate = 0.4;

    setColorFunctor(DefaultColorFunctor());
    setChangeInAngularVelocityFunctor(DefaultAngularVelocityFunctor());
    setChangeInVelocityFunctor(DefaultVelocityFunctor());
    setChangeInScaleFunctor(DefaultScaleFunctor());
    setDepthFunctor(DefaultDepthFunctor());

    setInitialVelocityFunctor(DefaultInitialVelocityFunctor());
    setInitialScaleFunctor(DefaultInitialScaleFunctor());
    setInitialAngularVelocityFunctor(DefaultInitialAngularVelocityFunctor());
}

ParticleEmissionProperties::ParticleEmissionProperties(Handle& materialHandle, const double lifeTime, const double spawnRate, const unsigned int ParticlesPerSpawn, const float drag) : ParticleEmissionProperties(){
    m_Lifetime = lifeTime;
    m_Drag = drag;
    m_SpawnRate = spawnRate;
    m_ParticleMaterials.push_back(Resources::getMaterial(materialHandle));
    m_ParticlesPerSpawn = ParticlesPerSpawn;
}
ParticleEmissionProperties::~ParticleEmissionProperties() {
    m_ParticleMaterials.clear();
}

ParticleEmissionProperties::ParticleEmissionProperties(const ParticleEmissionProperties& other) {
    
    m_Lifetime = other.m_Lifetime;
    m_Drag = other.m_Drag;
    m_SpawnRate = other.m_SpawnRate;
    m_ParticlesPerSpawn = other.m_ParticlesPerSpawn;
    m_ParticleMaterials = other.m_ParticleMaterials;

    
    m_ColorFunctor = other.m_ColorFunctor;
    m_ChangeInAngularVelocityFunctor = other.m_ChangeInAngularVelocityFunctor;
    m_ChangeInVelocityFunctor = other.m_ChangeInVelocityFunctor;
    m_ChangeInScaleFunctor = other.m_ChangeInScaleFunctor;
    m_DepthFunctor = other.m_DepthFunctor;
    m_InitialVelocityFunctor = other.m_InitialVelocityFunctor;
    m_InitialScaleFunctor = other.m_InitialScaleFunctor;
    m_InitialAngularVelocityFunctor = other.m_InitialAngularVelocityFunctor;
    

}
ParticleEmissionProperties& ParticleEmissionProperties::operator=(const ParticleEmissionProperties& other) {
    if (&other == this)
        return *this;
    m_Lifetime = other.m_Lifetime;
    m_Drag = other.m_Drag;
    m_SpawnRate = other.m_SpawnRate;
    m_ParticlesPerSpawn = other.m_ParticlesPerSpawn;
    m_ParticleMaterials = other.m_ParticleMaterials;

    
    m_ColorFunctor = other.m_ColorFunctor;
    m_ChangeInAngularVelocityFunctor = other.m_ChangeInAngularVelocityFunctor;
    m_ChangeInVelocityFunctor = other.m_ChangeInVelocityFunctor;
    m_ChangeInScaleFunctor = other.m_ChangeInScaleFunctor;
    m_DepthFunctor = other.m_DepthFunctor;
    m_InitialVelocityFunctor = other.m_InitialVelocityFunctor;
    m_InitialScaleFunctor = other.m_InitialScaleFunctor;
    m_InitialAngularVelocityFunctor = other.m_InitialAngularVelocityFunctor;
    
    return *this;
}
ParticleEmissionProperties::ParticleEmissionProperties(ParticleEmissionProperties&& other) noexcept {
    using std::swap;
    swap(m_Lifetime, other.m_Lifetime);
    swap(m_Drag, other.m_Drag);
    swap(m_SpawnRate, other.m_SpawnRate);
    swap(m_ParticlesPerSpawn, other.m_ParticlesPerSpawn);
    swap(m_ParticleMaterials, other.m_ParticleMaterials);

    
    swap(m_ColorFunctor, other.m_ColorFunctor);
    swap(m_ChangeInAngularVelocityFunctor, other.m_ChangeInAngularVelocityFunctor);
    swap(m_ChangeInVelocityFunctor, other.m_ChangeInVelocityFunctor);
    swap(m_ChangeInScaleFunctor, other.m_ChangeInScaleFunctor);
    swap(m_DepthFunctor, other.m_DepthFunctor);
    swap(m_InitialVelocityFunctor, other.m_InitialVelocityFunctor);
    swap(m_InitialScaleFunctor, other.m_InitialScaleFunctor);
    swap(m_InitialAngularVelocityFunctor, other.m_InitialAngularVelocityFunctor);
    
}
ParticleEmissionProperties& ParticleEmissionProperties::operator=(ParticleEmissionProperties&& other) noexcept {
    using std::swap;
    swap(m_Lifetime, other.m_Lifetime);
    swap(m_Drag, other.m_Drag);
    swap(m_SpawnRate, other.m_SpawnRate);
    swap(m_ParticlesPerSpawn, other.m_ParticlesPerSpawn);
    swap(m_ParticleMaterials, other.m_ParticleMaterials);


    swap(m_ColorFunctor, other.m_ColorFunctor);
    swap(m_ChangeInAngularVelocityFunctor, other.m_ChangeInAngularVelocityFunctor);
    swap(m_ChangeInVelocityFunctor, other.m_ChangeInVelocityFunctor);
    swap(m_ChangeInScaleFunctor, other.m_ChangeInScaleFunctor);
    swap(m_DepthFunctor, other.m_DepthFunctor);
    swap(m_InitialVelocityFunctor, other.m_InitialVelocityFunctor);
    swap(m_InitialScaleFunctor, other.m_InitialScaleFunctor);
    swap(m_InitialAngularVelocityFunctor, other.m_InitialAngularVelocityFunctor);
    return *this;
}

const double& ParticleEmissionProperties::getLifetime() const {
    return m_Lifetime;
}
const double& ParticleEmissionProperties::getSpawnRate() const {
    return m_SpawnRate;
}

const bool ParticleEmissionProperties::addMaterial(Material& material) {
    for (auto& mat : m_ParticleMaterials) {
        if (&material == mat)
            return false;
    }
    m_ParticleMaterials.push_back(&material);
    return true;
}
const bool ParticleEmissionProperties::addMaterial(Handle& materialHandle) {
    return ParticleEmissionProperties::addMaterial(*Resources::getMaterial(materialHandle));
}

const Material& ParticleEmissionProperties::getParticleMaterialRandom() const {
    if (m_ParticleMaterials.size() == 0)
        return *Material::Checkers;
    const auto index = rand() % m_ParticleMaterials.size();
    return *m_ParticleMaterials[index];
}
const Material& ParticleEmissionProperties::getParticleMaterial(const size_t index) const {
    if (m_ParticleMaterials.size() == 0)
        return *Material::Checkers;
    return *m_ParticleMaterials[index];
}
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>

using namespace std;
using namespace Engine;

ParticleEmissionProperties::ParticleEmissionProperties(Handle& materialHandle, const float lifeTime, const float spawnRate, const unsigned int ParticlesPerSpawn, const float drag){
    m_Lifetime          = lifeTime;
    m_SpawnRate         = spawnRate;
    m_ParticlesPerSpawn = ParticlesPerSpawn;
    m_ParticleMaterials.push_back(materialHandle.get<Material>());
}

ParticleEmissionProperties::ParticleEmissionProperties(ParticleEmissionProperties&& other) noexcept {
    m_Lifetime                       = std::move(other.m_Lifetime);
    m_SpawnRate                      = std::move(other.m_SpawnRate);
    m_ParticlesPerSpawn              = std::move(other.m_ParticlesPerSpawn);
    m_ParticleMaterials              = std::move(other.m_ParticleMaterials);
    m_ColorFunctor                   = std::move(other.m_ColorFunctor);
    m_ChangeInAngularVelocityFunctor = std::move(other.m_ChangeInAngularVelocityFunctor);
    m_ChangeInVelocityFunctor        = std::move(other.m_ChangeInVelocityFunctor);
    m_ChangeInScaleFunctor           = std::move(other.m_ChangeInScaleFunctor);
    m_InitialVelocityFunctor         = std::move(other.m_InitialVelocityFunctor);
    m_InitialScaleFunctor            = std::move(other.m_InitialScaleFunctor);
    m_InitialAngularVelocityFunctor  = std::move(other.m_InitialAngularVelocityFunctor);
}
ParticleEmissionProperties& ParticleEmissionProperties::operator=(ParticleEmissionProperties&& other) noexcept {
    m_Lifetime                       = std::move(other.m_Lifetime);
    m_SpawnRate                      = std::move(other.m_SpawnRate);
    m_ParticlesPerSpawn              = std::move(other.m_ParticlesPerSpawn);
    m_ParticleMaterials              = std::move(other.m_ParticleMaterials);
    m_ColorFunctor                   = std::move(other.m_ColorFunctor);
    m_ChangeInAngularVelocityFunctor = std::move(other.m_ChangeInAngularVelocityFunctor);
    m_ChangeInVelocityFunctor        = std::move(other.m_ChangeInVelocityFunctor);
    m_ChangeInScaleFunctor           = std::move(other.m_ChangeInScaleFunctor);
    m_InitialVelocityFunctor         = std::move(other.m_InitialVelocityFunctor);
    m_InitialScaleFunctor            = std::move(other.m_InitialScaleFunctor);
    m_InitialAngularVelocityFunctor  = std::move(other.m_InitialAngularVelocityFunctor);
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
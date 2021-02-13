
#include <serenity/renderer/particles/ParticleEmissionProperties.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/material/Material.h>

ParticleEmissionProperties::ParticleEmissionProperties(Handle materialHandle, float lifeTime, float spawnRate, unsigned int ParticlesPerSpawn, float drag)
    : m_Lifetime{ lifeTime }
    , m_SpawnRate{ spawnRate }
    , m_ParticlesPerSpawn{ ParticlesPerSpawn }
{
    m_ParticleMaterials.emplace_back(materialHandle.get<Material>());
}

bool ParticleEmissionProperties::addMaterial(Material& material) {
    for (const auto& mat : m_ParticleMaterials) {
        if (&material == mat) {
            return false;
        }
    }
    m_ParticleMaterials.emplace_back(&material);
    return true;
}
bool ParticleEmissionProperties::addMaterial(Handle materialHandle) {
    return ParticleEmissionProperties::addMaterial(*Engine::Resources::getResource<Material>(materialHandle));
}
const Material& ParticleEmissionProperties::getParticleMaterialRandom() const noexcept {
    return (m_ParticleMaterials.size() == 0) ? *Material::Checkers.get<Material>() : *m_ParticleMaterials[rand() % m_ParticleMaterials.size()];
}
const Material& ParticleEmissionProperties::getParticleMaterial(size_t index) const noexcept {
    return (m_ParticleMaterials.size() == 0) ? *Material::Checkers.get<Material>() : *m_ParticleMaterials[index];
}
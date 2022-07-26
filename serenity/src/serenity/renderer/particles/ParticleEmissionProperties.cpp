
#include <serenity/renderer/particles/ParticleEmissionProperties.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/material/Material.h>

ParticleEmissionProperties::ParticleEmissionProperties(Handle materialHandle, float lifeTime, float spawnRate, uint8_t ParticlesPerSpawn, float drag)
    : m_Lifetime{ lifeTime }
    , m_SpawnRate{ spawnRate }
    , m_ParticlesPerSpawn{ ParticlesPerSpawn }
{
    m_ParticleMaterials.emplace_back(materialHandle.get<Material>());
}
bool ParticleEmissionProperties::addMaterial(Material& inMaterial) {
    for (const auto& material : m_ParticleMaterials) {
        if (&inMaterial == material) {
            return false;
        }
    }
    m_ParticleMaterials.emplace_back(&inMaterial);
    return true;
}
bool ParticleEmissionProperties::addMaterial(Handle materialHandle) {
    return ParticleEmissionProperties::addMaterial(*Engine::Resources::getResource<Material>(materialHandle));
}
Material& ParticleEmissionProperties::getParticleMaterialRandom() const noexcept {
    const size_t randomIndex = static_cast<size_t>(rand()) % m_ParticleMaterials.size();
    assert(randomIndex >= 0 && randomIndex < m_ParticleMaterials.size());
    return (m_ParticleMaterials.size() == 0) ? *Material::Checkers.get<Material>() : *m_ParticleMaterials[randomIndex];
}
Material& ParticleEmissionProperties::getParticleMaterial(size_t index) const noexcept {
    assert(index >= 0 && index < m_ParticleMaterials.size());
    return (m_ParticleMaterials.size() == 0) ? *Material::Checkers.get<Material>() : *m_ParticleMaterials[index];
}
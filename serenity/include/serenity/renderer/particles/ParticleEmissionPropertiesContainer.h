#pragma once
#ifndef ENGINE_PARTICLE_EMISSION_PROPERTIES_CONTAINER_H
#define ENGINE_PARTICLE_EMISSION_PROPERTIES_CONTAINER_H

class ParticleEmissionProperties;
class Scene;
namespace Engine::priv {
	class ParticleEmissionPropertiesContainer;
	class ParticleSystemNew;
}

#include <serenity/renderer/particles/ParticleEmissionPropertiesHandle.h>
#include <unordered_map>
#include <serenity/types/ViewPointer.h>

namespace Engine::priv {
	class ParticleEmissionPropertiesContainer {
	    public:
			using VectorContainer = std::vector<ParticleEmissionProperties>;
		private:
			std::vector<ParticleEmissionProperties> m_Properties;
			std::vector<uint32_t>                   m_MappingBackwards;
			std::vector<uint32_t>                   m_Mapping;
			size_t                                  m_LastIndex = 0;
		public:
			ParticleEmissionPropertiesContainer() = default;
			ParticleEmissionPropertiesContainer(size_t capacity);

			ParticleEmissionPropertiesHandle addProperties(const ParticleEmissionProperties&);
			ParticleEmissionPropertiesHandle addProperties(Handle materialHandle, float lifeTime, float spawnRate, uint8_t ParticlesPerSpawn = 1, float drag = 1.0f);

			[[nodiscard]] Engine::view_ptr<ParticleEmissionProperties> getProperties(ParticleEmissionPropertiesHandle) noexcept;

			[[nodiscard]] inline typename VectorContainer::iterator begin() noexcept { return m_Properties.begin(); }
			[[nodiscard]] inline typename VectorContainer::const_iterator begin() const noexcept { return m_Properties.begin(); }
			[[nodiscard]] inline typename VectorContainer::iterator end() noexcept { return m_Properties.begin() + m_LastIndex; }
			[[nodiscard]] inline typename VectorContainer::const_iterator end() const noexcept { return m_Properties.begin() + m_LastIndex; }
			[[nodiscard]] inline typename const VectorContainer::const_iterator cbegin() const noexcept { return m_Properties.cbegin(); }
			[[nodiscard]] inline typename const VectorContainer::const_iterator cend() const noexcept { return m_Properties.cbegin() + m_LastIndex; }
	};
}

#endif
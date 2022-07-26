#include <serenity/renderer/particles/ParticleEmissionPropertiesContainer.h>

namespace {
	static constexpr uint32_t NULL_IDX = std::numeric_limits<uint32_t>().max();

	void interal_set_capacity(size_t capacity, auto& emitters, auto& mapping, auto& mappingBackwards) {
		size_t oldCapacity = emitters.size();
		emitters.reserve(capacity);
		mapping.reserve(capacity);
		mappingBackwards.reserve(capacity);
		for (size_t i = oldCapacity; i < capacity; ++i) {
			mappingBackwards.emplace_back(uint32_t(i));
			mapping.emplace_back(uint32_t(i));
		}
	}
}



Engine::priv::ParticleEmissionPropertiesContainer::ParticleEmissionPropertiesContainer(size_t capacity) {
	interal_set_capacity(capacity, m_Properties, m_Mapping, m_MappingBackwards);
}

Engine::view_ptr<ParticleEmissionProperties> Engine::priv::ParticleEmissionPropertiesContainer::getProperties(ParticleEmissionPropertiesHandle handle) noexcept {
	return m_Mapping[handle.id()] == NULL_IDX ? nullptr : &m_Properties[m_Mapping[handle.id()]];
}
ParticleEmissionPropertiesHandle Engine::priv::ParticleEmissionPropertiesContainer::addProperties(const ParticleEmissionProperties& properties) {
	if (m_Properties.capacity() == m_LastIndex) {
		interal_set_capacity(m_Properties.size() == 0 ? 1 : m_Properties.size() * 2, m_Properties, m_Mapping, m_MappingBackwards);
	}
	const auto freeIdx = m_MappingBackwards[m_LastIndex];
	m_Mapping[freeIdx] = uint32_t(m_LastIndex);
	if (m_Properties.size() == m_LastIndex) {
		m_Properties.emplace_back(properties);
	} else {
		m_Properties[m_LastIndex] = properties;
	}
	++m_LastIndex;
	return ParticleEmissionPropertiesHandle{ freeIdx };
}
ParticleEmissionPropertiesHandle Engine::priv::ParticleEmissionPropertiesContainer::addProperties(Handle materialHandle, float lifeTime, float spawnRate, uint8_t ParticlesPerSpawn, float drag) {
	if (m_Properties.capacity() == m_LastIndex) {
		interal_set_capacity(m_Properties.size() == 0 ? 1 : m_Properties.size() * 2, m_Properties, m_Mapping, m_MappingBackwards);
	}
	const auto freeIdx = m_MappingBackwards[m_LastIndex];
	m_Mapping[freeIdx] = uint32_t(m_LastIndex);
	if (m_Properties.size() == m_LastIndex) {
		m_Properties.emplace_back(materialHandle, lifeTime, spawnRate, ParticlesPerSpawn, drag);
	} else {
		m_Properties[m_LastIndex] = ParticleEmissionProperties{ materialHandle, lifeTime, spawnRate, ParticlesPerSpawn, drag };
	}
	++m_LastIndex;
	return ParticleEmissionPropertiesHandle{ freeIdx };
}
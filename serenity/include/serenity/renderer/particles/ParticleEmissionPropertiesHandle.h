#pragma once
#ifndef ENGINE_RENDERER_PARTICLES_PARTICLE_EMISSION_PROPERTIES_HANDLE_H
#define ENGINE_RENDERER_PARTICLES_PARTICLE_EMISSION_PROPERTIES_HANDLE_H

class Material;
namespace Engine::priv {
	class ParticleSystemNew;
	class ParticleEmissionPropertiesContainer;
}

#include <serenity/resources/Handle.h>
#include <serenity/renderer/particles/ParticleEmissionProperties.h>

class ParticleEmissionPropertiesHandle {
	friend class Engine::priv::ParticleSystemNew;
	friend class Engine::priv::ParticleEmissionPropertiesContainer;
	private:
		uint32_t m_ID = std::numeric_limits<uint32_t>().max();

		ParticleEmissionPropertiesHandle(uint32_t id)
			: m_ID{ id }
		{}
	public:
		ParticleEmissionPropertiesHandle() = default;
		[[nodiscard]] inline uint32_t id() const noexcept { return m_ID; }
		[[nodiscard]] inline bool null() const noexcept { return m_ID == std::numeric_limits<uint32_t>().max(); }
		[[nodiscard]] inline explicit operator bool() const noexcept { return !null(); }
		//[[nodiscard]] inline bool operator==(const ParticleEmissionPropertiesHandle& other) const noexcept { m_ID == other.m_ID; }

		[[nodiscard]] float getTimer() const noexcept;
		[[nodiscard]] float getLifetime() const noexcept;
		[[nodiscard]] float getSpawnRate() const noexcept;
		[[nodiscard]] float getSpawnTimer() const noexcept;
		[[nodiscard]] Engine::view_ptr<Material> getParticleMaterialRandom() noexcept;
		[[nodiscard]] Engine::view_ptr<Material> getParticleMaterial(size_t index = 0) noexcept;

		bool addMaterial(Handle materialHandle);
		bool addMaterial(Material&);

		bool setSpawnTimer(const float amount) noexcept;
		bool setColorFunctor(ParticleEmissionProperties::color_func&& functor) noexcept;
		bool setChangeInAngularVelocityFunctor(ParticleEmissionProperties::change_in_angular_velocity_func&& functor) noexcept;
		bool setChangeInVelocityFunctor(ParticleEmissionProperties::change_in_velocity_func&& functor) noexcept;
		bool setChangeInScaleFunctor(ParticleEmissionProperties::change_in_scale_func&& functor) noexcept;
		bool setInitialVelocityFunctor(ParticleEmissionProperties::initial_velocity_func&& functor) noexcept;
		bool setInitialScaleFunctor(ParticleEmissionProperties::initial_scale_func&& functor) noexcept;
		bool setInitialAngularVelocityFunctor(ParticleEmissionProperties::initial_angular_velocity_func&& functor) noexcept;
};

namespace std {
	template <>
	struct hash<ParticleEmissionPropertiesHandle> {
		inline size_t operator()(const ParticleEmissionPropertiesHandle& x) const {
			return hash<uint32_t>()(x.id());
		}
	};
}

#endif
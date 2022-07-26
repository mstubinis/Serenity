#pragma once
#ifndef ENGINE_RENDERER_PARTICLES_PARTICLE_EMITTER_HANDLE_H
#define ENGINE_RENDERER_PARTICLES_PARTICLE_EMITTER_HANDLE_H

class Scene;
namespace Engine::priv {
	class ParticleSystemNew;
}

#include <serenity/dependencies/glm.h>

class ParticleEmitterHandle {
	friend class Engine::priv::ParticleSystemNew;
	private:
		uint32_t m_ID      = std::numeric_limits<uint32_t>().max();
		uint32_t m_SceneID = std::numeric_limits<uint32_t>().max();
	public:
		constexpr ParticleEmitterHandle() = default;
		ParticleEmitterHandle(uint32_t id, uint32_t sceneID);
		ParticleEmitterHandle(uint32_t id, Scene&);

		[[nodiscard]] inline constexpr uint32_t id() const noexcept { return m_ID; }
		[[nodiscard]] inline constexpr uint32_t sceneID() const noexcept { return m_SceneID; }
		[[nodiscard]] inline constexpr bool null() const noexcept { return m_ID == std::numeric_limits<uint32_t>().max() && m_SceneID == std::numeric_limits<uint32_t>().max(); }
		[[nodiscard]] inline constexpr explicit operator bool() const noexcept { return !null(); }

		void setPosition(decimal x, decimal y, decimal z);
		void setPosition(const glm_vec3& position);
};

namespace std {
	template <>
	struct hash<ParticleEmitterHandle> {
		inline size_t operator()(const ParticleEmitterHandle& x) const {
			return std::hash<uint32_t>()(x.id());
		}
	};

	template<>
	struct equal_to<ParticleEmitterHandle> {
		bool operator()(const ParticleEmitterHandle& lhs, const ParticleEmitterHandle& rhs) const {
			return lhs.id() == rhs.id() && lhs.sceneID() == rhs.sceneID();
		}
	};
	
}

#endif
#pragma once
#ifndef ENGINE_RENDERER_PARTICLES_PARTICLE_HANDLE_H
#define ENGINE_RENDERER_PARTICLES_PARTICLE_HANDLE_H

#include <cstdint>
#include <numeric>

class ParticleHandle {
	public:
		inline static constexpr uint32_t NULL_ID = std::numeric_limits<uint32_t>().max();
	private:
		uint32_t m_ID = NULL_ID;

	public:
		constexpr ParticleHandle() = default;
		constexpr ParticleHandle(uint32_t id)
			: m_ID{ id }
		{}

		//[[nodiscard]] inline bool operator==(const ParticleHandle& other) noexcept { return m_ID == other.m_ID; }

		[[nodiscard]] inline constexpr uint32_t id() const noexcept { return m_ID; }
		[[nodiscard]] inline constexpr bool null() const noexcept { return m_ID == NULL_ID; }
		[[nodiscard]] inline constexpr explicit operator bool() const noexcept { return !null(); }
};
namespace std {
	template <> 
	struct hash<ParticleHandle> {
		inline size_t operator()(const ParticleHandle& x) const {
			return std::hash<uint32_t>()(x.id());
		}
	};

	template<>
	struct equal_to<ParticleHandle> {
		constexpr bool operator()(const ParticleHandle& lhs, const ParticleHandle& rhs) const {
			return lhs.id() == rhs.id();
		}
	};
}
#endif
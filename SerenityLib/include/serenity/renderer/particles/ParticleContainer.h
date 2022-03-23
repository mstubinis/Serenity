#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_CONTAINER_H
#define ENGINE_RENDERER_PARTICLE_CONTAINER_H

class Camera;
class Material;
class ParticleEmitter;

#include <serenity/containers/DODMembers.h>
#include <serenity/dependencies/glm.h>

using ParticleID = size_t;

namespace Engine::priv {
	class ParticleContainer {
		private:
			Engine::DODMembers<
				  glm::vec<3, ParticleFloatType, glm::packed_highp> //positions        [0]
				, glm::vec<2, ParticleFloatType, glm::packed_highp> //scaleXY          [1]
				, ParticleFloatType                                 //angle            [2]
				, ParticleIDType                                    //matID            [3]
				, ParticleIDType                                    //packedColor      [4]
				, glm::vec3                                         //velocity         [5]
				, float                                             //angularVelocity  [6]
				, float                                             //timer            [7]
				, ParticleID                                        //particle id      [8]
				, ParticleID                                        //particle id map  [9]
				//, Material*                                                          [10]
				//, ParticleEmitter*                                                   [11]
			> m_ParticlesData;

			size_t m_ActiveEnd = 0;
	    private:
			template<size_t INDEX>
			[[nodiscard]] inline auto& getDataMember(ParticleID id) noexcept { return m_ParticlesData.get<INDEX>(m_ParticlesData.get<9>(id)); }
			template<size_t INDEX>
			[[nodiscard]] inline const auto& getDataMember(ParticleID id) const noexcept { return m_ParticlesData.get<INDEX>(m_ParticlesData.get<9>(id)); }

			inline void remapIndex(size_t index) noexcept { m_ParticlesData.get<9>(m_ParticlesData.get<8>(index)) = index; }
		public:
			ParticleContainer() = delete;
			ParticleContainer(uint32_t maxParticles);
			ParticleContainer(const ParticleContainer&)                = delete;
			ParticleContainer& operator=(const ParticleContainer&)     = delete;
			ParticleContainer(ParticleContainer&&) noexcept            = delete;
			ParticleContainer& operator=(ParticleContainer&&) noexcept = delete;

			bool recycleParticle(ParticleID);
			ParticleID addParticle(
				float x, 
				float y, 
				float z, 
				float scaleX, 
				float scaleY, 
				ParticleIDType matID,
				uint8_t r = 255,
				uint8_t g = 255,
				uint8_t b = 255,
				uint8_t a = 255, 
				const glm::vec3& velocity = glm::vec3{ 0.0f },
				float angularVelocity = 0.0f
			);

			void sort_particles(const Camera&);
			void sort_particles(const glm::vec3& cameraPosition);

			void translate(ParticleID, const glm::vec3& translation) noexcept;
			void translate(ParticleID, float x, float y, float z) noexcept;

			void setScale(ParticleID, const glm::vec2& newScale) noexcept;
			void setScale(ParticleID, float x, float y) noexcept;

			void setPosition(ParticleID, const glm::vec3& newPosition) noexcept;
			void setPosition(ParticleID, float x, float y, float z) noexcept;

			void setColor(ParticleID, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
			void setVelocity(ParticleID, float vx, float vy, float vz);
			void setAngularVelocity(ParticleID, float av);

			void setTimer(ParticleID, float time);

			[[nodiscard]] glm::vec3 getPosition(ParticleID) const noexcept;
			//[[nodiscard]] glm::vec3 getPositionFromIndex(size_t) const noexcept;

			[[nodiscard]] inline size_t getNumParticles() const noexcept { return m_ParticlesData.size(); }
			[[nodiscard]] inline size_t getActiveEnd() const noexcept { return m_ActiveEnd; }

			/*
		void init(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& parent) noexcept;

		[[nodiscard]] inline bool isActive() const noexcept { return (m_Timer > 0.0f); }
		[[nodiscard]] inline Material* getMaterial() const noexcept { return m_Material; }
		[[nodiscard]] inline float angle() const noexcept { return m_Angle; }
		[[nodiscard]] inline const glm::vec2& getScale() const noexcept { return m_Scale; }
		[[nodiscard]] inline const glm::vec3& position() const noexcept { return m_Position; }
		[[nodiscard]] inline const Engine::color_vector_4& color() const noexcept { return m_Color; }
		[[nodiscard]] inline const glm::vec3& velocity() const noexcept { return m_Velocity; }
		[[nodiscard]] float lifetime() const noexcept;

			*/
	};
}

#endif
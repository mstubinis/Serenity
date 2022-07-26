#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_CONTAINER_H
#define ENGINE_RENDERER_PARTICLE_CONTAINER_H

class Camera;
class ParticleEmissionProperties;
namespace Engine::priv{
	class ParticleSystemNew;
}

#include <serenity/renderer/particles/ParticleIncludes.h>
#include <serenity/containers/DODMembers.h>
#include <serenity/dependencies/glm.h>
#include <serenity/renderer/particles/ParticleHandle.h>
#include <serenity/renderer/particles/ParticleEmissionPropertiesHandle.h>
#include <serenity/algorithms/RadixSort.h>

namespace Engine::priv {
	class ParticleContainer {
		friend class Engine::priv::ParticleSystemNew;
		public:
			struct ParticleIDMapping {
				size_t id = std::numeric_limits<size_t>().max();
			};

			/*
	"layout (location = 0) in vec3 aVertexPosition;\n"
	"layout (location = 1) in vec2 aUV;\n"

	"layout (location = 2) in vec4 aParticlePositionAndScaleX;\n"
	"layout (location = 3) in vec2 aParticleScaleYAndRotation;\n"
	"layout (location = 4) in uvec2 aParticleMaterialIndexAndColorPacked;\n"
			*/
			struct Particle {
				glm::vec<3, ParticleFloatType, glm::packed_highp> m_position; //update loop
				glm::vec<2, ParticleFloatType, glm::packed_highp> m_scale; //update loop
				ParticleFloatType                                 m_angle; //update loop
				ParticlePackedColorType                           m_packedColor; //update loop
				glm::vec<3, ParticleFloatType, glm::packed_highp> m_velocity; //update loop, NOT shader
				ParticleEmissionPropertiesHandle                  m_propHandle; //update loop, NOT shader
				float                                             m_angularVelocity; //update loop, NOT shader
				float                                             m_timer; //update loop, NOT shader

				Particle() = default;
				Particle(
					const glm::vec<3, ParticleFloatType, glm::packed_highp>& position, 
					const glm::vec<2, ParticleFloatType, glm::packed_highp>& scale, 
					ParticleFloatType angle,
					ParticlePackedColorType packedColor, 
					const glm::vec<3, ParticleFloatType, glm::packed_highp>& velocity,
					float angularVelocity, 
					float timer
				)
					: m_position{ position }
					, m_scale{ scale }
					, m_angle{ angle }
					, m_packedColor{ packedColor }
					, m_velocity{ velocity }
					, m_angularVelocity{ angularVelocity }
					, m_timer{ timer }
				{
				}
			};
		private:
			//Engine::algorithms::float_sort_parallel<2048> m_Float_Sorter;
			Engine::unordered_bimap<Material*, uint32_t>                       m_Bimap;

			std::vector<Particle>               m_ParticlesData;
			std::vector<ParticleMaterialIDType> m_MaterialIDs;  //YES TO SHADER	

			std::vector<std::size_t>            m_CullingAuxVector;
			std::vector<bool>                   m_CullingAuxVector2;
			std::vector<float>                  m_SortingDistances;


			size_t     m_ActiveEnd = 0;
		public:
			ParticleContainer() = delete;
			ParticleContainer(uint32_t maxParticles);
			ParticleContainer(const ParticleContainer&)                = delete;
			ParticleContainer& operator=(const ParticleContainer&)     = delete;
			ParticleContainer(ParticleContainer&&) noexcept            = delete;
			ParticleContainer& operator=(ParticleContainer&&) noexcept = delete;

			[[nodiscard]] inline Particle& operator[](size_t idx) noexcept { return m_ParticlesData[idx]; }
			[[nodiscard]] inline const Particle& operator[](size_t idx) const noexcept { return m_ParticlesData[idx]; }
			[[nodiscard]] inline size_t size() const noexcept { return m_ActiveEnd; }
			[[nodiscard]] inline bool empty() const noexcept { return size() == 0; }


			bool recycleParticle(size_t particleIndex);

			bool addParticle(const ParticleEmissionProperties&, ParticleEmissionPropertiesHandle, const glm_vec3& emitterPosition, const glm::quat& emitterRotation,
				float scaleX, float scaleY, ParticleIDType matID, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const glm::vec3& velocity, float angularVelocity);

			bool addParticle(
				ParticleEmissionPropertiesHandle,
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

			//partitions the collection. left side = needed particles, right side = inactive and culled particles
			size_t cull_particles(const Camera&);

			void sort_particles(const Camera&, size_t numParticles);
			void sort_particles(const glm::vec3& cameraPosition, size_t numParticles);
			void associate_materials();

			[[nodiscard]] inline size_t getNumParticles() const noexcept { return m_ParticlesData.size(); }
			[[nodiscard]] inline size_t getActiveEnd() const noexcept { return m_ActiveEnd; }

			[[nodiscard]] inline auto getDataPtr() noexcept { return m_ParticlesData.data(); }
			[[nodiscard]] inline auto getMatID(size_t idx) noexcept { return m_MaterialIDs[idx]; }

			[[nodiscard]] inline typename std::vector<Particle>::iterator begin() noexcept { return m_ParticlesData.begin(); }
			[[nodiscard]] inline typename std::vector<Particle>::const_iterator begin() const noexcept { return m_ParticlesData.begin(); }
			[[nodiscard]] inline typename std::vector<Particle>::iterator end() noexcept { return m_ParticlesData.begin() + m_ActiveEnd; }
			[[nodiscard]] inline typename std::vector<Particle>::const_iterator end() const noexcept { return m_ParticlesData.begin() + m_ActiveEnd; }
			[[nodiscard]] inline typename const std::vector<Particle>::const_iterator cbegin() const noexcept { return m_ParticlesData.cbegin(); }
			[[nodiscard]] inline typename const std::vector<Particle>::const_iterator cend() const noexcept { return m_ParticlesData.cbegin() + m_ActiveEnd; }
	};
}

#endif
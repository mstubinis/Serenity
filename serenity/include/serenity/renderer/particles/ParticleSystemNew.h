#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_SYSTEM_NEW_H
#define ENGINE_RENDERER_PARTICLE_SYSTEM_NEW_H

class  Entity;
class  ParticleEmissionProperties;
class  Scene;
class  Camera;
class  ShaderProgram;
class  Material;
class  Viewport;
class  ParticleEmitter;
namespace Engine::priv {
	class RenderModule;
	struct PublicScene;
};

#include <serenity/renderer/particles/ParticleContainer.h>
#include <serenity/renderer/particles/ParticleEmitterContainer.h>
#include <serenity/renderer/particles/ParticleEmissionPropertiesHandle.h>
#include <mutex>

namespace Engine::priv {
	class ParticleSystemNew {
		friend struct Engine::priv::PublicScene;	
		private:
			ParticleContainer                                                  m_ParticleContainer;
			ParticleEmitterContainer<ParticleEmitter, ParticleEmitterHandle>   m_ParticleEmitterContainer;
			mutable std::mutex  m_Mutex;

			void internal_update_properties(ParticleEmissionProperties&, const float dt);
			void internal_update_emitter(ParticleEmitter&, size_t emitterIndex, const float dt);
			void internal_update_particle(Engine::priv::ParticleContainer::Particle&, size_t particleIndex, const float dt);

			ParticleSystemNew() = delete;
		public:
			ParticleSystemNew(uint32_t maxEmitters, uint32_t maxParticles);
			ParticleSystemNew(const ParticleSystemNew&) = delete;
			ParticleSystemNew& operator=(const ParticleSystemNew&) = delete;
			ParticleSystemNew(ParticleSystemNew&&) noexcept = delete;
			ParticleSystemNew& operator=(ParticleSystemNew&&) noexcept = delete;

			ParticleEmitterHandle add_emitter(ParticleEmissionPropertiesHandle, Scene&, float lifetime);

			void update(const float dt);
			void render(Viewport&, Camera&, RenderModule&);
	};
}

#endif
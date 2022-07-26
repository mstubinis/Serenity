#include <serenity/renderer/particles/ParticleSystemNew.h>
#include <serenity/resources/material/Material.h>

#include <serenity/resources/Engine_Resources.h>


#include <serenity/utils/BlockProfiler.h>

namespace {
	void internal_update_properties_after(ParticleEmissionProperties& prop) {
		if (prop.getSpawnTimer() > prop.getSpawnRate()) {
			prop.setSpawnTimer(0.0f);
		}
	}
}

Engine::priv::ParticleSystemNew::ParticleSystemNew(uint32_t maxEmitters, uint32_t maxParticles)
	: m_ParticleContainer{ maxParticles }
	, m_ParticleEmitterContainer{ maxEmitters }
{
}
void Engine::priv::ParticleSystemNew::update(const float dt) {
	auto jobEmitter = [dt, this](size_t emitterIndex, int32_t jobIndex) {
		ParticleEmitter& emitter = m_ParticleEmitterContainer[emitterIndex];
		internal_update_emitter(emitter, emitterIndex, dt);
	};
	auto jobParticle = [dt, this](size_t particleIndex, int32_t jobIndex) {
		Engine::priv::ParticleContainer::Particle& particle = m_ParticleContainer[particleIndex];
		internal_update_particle(particle, particleIndex, dt);
	};
	auto& properties = Engine::getResourceManager().getParticleProperties();

	for (auto& prop : properties) {
		internal_update_properties(prop, dt);
	}
	{
		//Engine::block_profiler blk = Engine::block_profiler(std::cout, "Update jobs (" + std::to_string(m_ParticleContainer.getNumParticles()) + "): ");
		Engine::priv::threading::addJobSplitVectored(jobEmitter, true, false, m_ParticleEmitterContainer.size(), 0);
		Engine::priv::threading::addJobSplitVectored(jobParticle, true, false, m_ParticleContainer.size(), 0);
	}
	//clean  up inactive particles
	for (size_t i = 0; i < m_ParticleContainer.size(); ++i) {
		const auto& particle  = m_ParticleContainer[i];
		const float currTimer = particle.m_timer;
		const auto& prop      = *Engine::Resources::getParticleEmissionProperties(particle.m_propHandle);
		if (currTimer >= prop.m_Lifetime) {
			if (m_ParticleContainer.recycleParticle(i)) {
				if (m_ParticleContainer.size() >= 2 && i > 0) {
					--i;
				}
			}
		}	
	}
	for (auto& prop : properties) {
		internal_update_properties_after(prop);
	}
	
}
void Engine::priv::ParticleSystemNew::render(Viewport& viewport, Camera& camera, RenderModule& renderModule) {
	if (!m_ParticleContainer.empty()) {
		//Engine::block_profiler block(std::cout, "Render: ");
		size_t numParticles = m_ParticleContainer.size();
		numParticles        = m_ParticleContainer.cull_particles(camera);
		//std::cout << lastIndex << '\n';
		m_ParticleContainer.sort_particles(camera, numParticles);
		m_ParticleContainer.associate_materials();
		//renderModule.renderParticles(numParticles);
	}
}
void Engine::priv::ParticleSystemNew::internal_update_properties(ParticleEmissionProperties& prop, const float dt) {
	prop.m_SpawningTimer += dt;
}
void Engine::priv::ParticleSystemNew::internal_update_emitter(ParticleEmitter& emitter, size_t emitterIndex, const float dt) {
	auto prop = *Engine::Resources::getParticleEmissionProperties(emitter.getProperties());
	if (emitter.isActive()) {
		emitter.incrementTimer(dt);
		if (prop.getSpawnTimer() > prop.getSpawnRate()) {
			const auto emitterTransform   = emitter.getComponent<ComponentTransform>();
			const auto emitterScale       = emitterTransform->getScale();
			const auto particleScale      = prop.m_InitialScaleFunctor() * Engine::Math::Max(emitterScale.x, emitterScale.y, emitterScale.z);
			auto velocity                 = prop.m_InitialVelocityFunctor() * emitterScale;
			auto color                    = prop.m_ColorFunctor(0.0f, prop.getLifetime(), 0.001f);
			std::scoped_lock lock{ m_Mutex };
			for (uint32_t i = 0; i < prop.m_ParticlesPerSpawn; ++i) {
				const auto& mat = prop.getParticleMaterialRandom();	
				const auto particleHandle = m_ParticleContainer.addParticle(
					prop, 
					emitter.getProperties(),
					emitterTransform->getWorldPosition(), 
					emitterTransform->getWorldRotation(), 
					particleScale.x, 
					particleScale.y, 
					ParticleIDType(mat.getID()), 
					color.rc(), 
					color.gc(), 
					color.bc(), 
					color.ac(), 
					velocity,
					prop.m_InitialAngularVelocityFunctor()
				);
			}
		}
		if (emitter.getLifetime() != 0.0f && emitter.getTimer() > emitter.getLifetime()) {
			emitter.deactivate(); //TODO: is this needed? try to emulate this behavior in m_ParticleEmitters.deactivate()
			std::scoped_lock lock{ m_Mutex };
			m_ParticleEmitterContainer.deactivate(emitterIndex);
		}
	}
}
void Engine::priv::ParticleSystemNew::internal_update_particle(Engine::priv::ParticleContainer::Particle& particle, size_t particleIndex, const float dt) {
	if (particle.m_timer > 0.0f) {
		const auto& prop            = *Engine::Resources::getParticleEmissionProperties(particle.m_propHandle);
		particle.m_timer           += dt;
		const float currTimer       = particle.m_timer;

		//scale
		auto scale                  = Engine::Math::ToFloatVec2(particle.m_scale);
		scale                      += prop.m_ChangeInScaleFunctor(dt);
		particle.m_scale            = Engine::Math::PackFloatVec2(scale);
		//color
		particle.m_packedColor      = prop.m_ColorFunctor(dt, prop.getLifetime(), currTimer).toPacked<decltype(particle.m_packedColor)>();
		//angular velocity
		particle.m_angularVelocity += prop.m_ChangeInAngularVelocityFunctor(dt);
		//angle
		auto angle                  = Engine::Math::ToFloat(particle.m_angle);
		angle                      += particle.m_angularVelocity * dt;
		particle.m_angle            = Engine::Math::PackFloat(angle);
		//velocity
		auto velocity               = Engine::Math::ToFloatVec3(particle.m_velocity);
		velocity                   += prop.m_ChangeInVelocityFunctor(dt);
		particle.m_velocity         = Engine::Math::PackFloatVec3(velocity);
		//position
		auto position               = Engine::Math::ToFloatVec3(particle.m_position);
		position                   += velocity * dt;
		particle.m_position         = Engine::Math::PackFloatVec3(position);
	}
}

ParticleEmitterHandle Engine::priv::ParticleSystemNew::add_emitter(ParticleEmissionPropertiesHandle propertyHandle, Scene& scene, float lifetime) {
	if (propertyHandle) {
		return m_ParticleEmitterContainer.add(scene, propertyHandle, lifetime, Entity{});
	}
	return ParticleEmitterHandle{};
}

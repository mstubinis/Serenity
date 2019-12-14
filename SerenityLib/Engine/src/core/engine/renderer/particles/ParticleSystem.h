#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_SYSTEM_H
#define ENGINE_RENDERER_PARTICLE_SYSTEM_H

#include <vector>
#include <stack>
#include <mutex>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

#define NUMBER_OF_PARTICLE_EMITTERS_LIMIT 2000
#define NUMBER_OF_PARTICLE_LIMIT 1000000

class ParticleEmitter;
class Particle;
class Camera;
class ShaderProgram;

namespace Engine {
    namespace epriv {
        class GBuffer;
        class ParticleSystem final{
            friend class ParticleEmitter;
            friend class Particle;
            private:
                std::vector<ParticleEmitter>           m_ParticleEmitters;
                std::vector<Particle>                  m_Particles;
                std::stack<size_t>                     m_ParticleEmitterFreelist;
                std::stack<size_t>                     m_ParticleFreelist;
                std::mutex                             m_Mutex;

                void internal_update_emitters(const double& dt);
                void internal_update_particles(const double& dt);
            public:
                ParticleSystem();
                ~ParticleSystem();

                ParticleSystem(const ParticleSystem&)                      = delete;
                ParticleSystem& operator=(const ParticleSystem&)           = delete;
                ParticleSystem(ParticleSystem&& other) noexcept            = delete;
                ParticleSystem& operator=(ParticleSystem&& other) noexcept = delete;


                ParticleEmitter* add_emitter(ParticleEmitter& emitter);
                const bool add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);
                const bool add_particle(ParticleEmitter& emitter);

                std::vector<ParticleEmitter>& getParticleEmitters();
                std::vector<Particle>&        getParticles();
                std::stack<size_t>&           getParticleEmittersFreelist();
                std::stack<size_t>&           getParticlesFreelist();

                void update(const double& dt);
                void render(Camera& camera, ShaderProgram& program, GBuffer& gBuffer);
            };
    };
};
#endif
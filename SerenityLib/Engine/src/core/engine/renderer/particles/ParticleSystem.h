#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_SYSTEM_H
#define ENGINE_RENDERER_PARTICLE_SYSTEM_H

class  ParticleEmitter;
class  ParticleEmissionProperties;
class  Scene;
struct Entity;
class  Particle;
class  Camera;
class  ShaderProgram;
class  Material;
class  Viewport;
namespace Engine::priv {
    class Renderer;
};

#include <vector>
#include <unordered_map>
#include <stack>
#include <mutex>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

constexpr unsigned int MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME = 12U;

#ifdef ENVIRONMENT64
    constexpr unsigned int NUMBER_OF_PARTICLE_EMITTERS_LIMIT = 4'000U;
    constexpr unsigned int NUMBER_OF_PARTICLE_LIMIT          = 1'000'000U;
#else
    constexpr unsigned int NUMBER_OF_PARTICLE_EMITTERS_LIMIT = 2'000U;
    constexpr unsigned int NUMBER_OF_PARTICLE_LIMIT = 500'000U;
#endif

namespace Engine::priv {
    class ParticleSystem final{
        friend class ParticleEmitter;
        friend class Particle;
        private:
            std::vector<ParticleEmitter>           m_ParticleEmitters;
            std::vector<Particle>                  m_Particles;

            std::stack<size_t>                     m_ParticleEmitterFreelist;
            std::stack<size_t>                     m_ParticleFreelist;
            std::mutex                             m_Mutex;

            void internal_update_emitters(const float dt);
            void internal_update_particles(const float dt, const Camera& camera);
        public:
            //DOD
            struct ParticleDOD final {
                glm::vec4 PositionAndScaleX;
                glm::vec2 ScaleYAndAngle;
                glm::uvec2 MatIDAndPackedColor;
            };

            std::vector<ParticleDOD>  ParticlesDOD;
            std::unordered_map<Material*, unsigned int>    MaterialToIndex;
            std::unordered_map<unsigned int, Material*>    MaterialToIndexReverse;
            std::unordered_map<unsigned int, unsigned int> MaterialIDToIndex;

            //for the threads...
            std::vector<std::vector<ParticleDOD>>                    THREAD_PART_1;
            std::vector<std::unordered_map<Material*, unsigned int>> THREAD_PART_4;
            std::vector<std::unordered_map<unsigned int, Material*>> THREAD_PART_5;
        public:
            ParticleSystem();
            ~ParticleSystem();

            ParticleSystem(const ParticleSystem&)                      = delete;
            ParticleSystem& operator=(const ParticleSystem&)           = delete;
            ParticleSystem(ParticleSystem&& other) noexcept            = delete;
            ParticleSystem& operator=(ParticleSystem&& other) noexcept = delete;


            ParticleEmitter* add_emitter(ParticleEmissionProperties& properties, Scene& scene, const float lifetime, const Entity parent);

            const bool add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);
            const bool add_particle(ParticleEmitter& emitter);

            std::vector<ParticleEmitter>& getParticleEmitters();
            std::vector<Particle>&        getParticles();
            std::stack<size_t>&           getParticleEmittersFreelist();
            std::stack<size_t>&           getParticlesFreelist();

            void update(const float dt, const Camera& camera);
            void render(const Viewport& viewport, const Camera& camera, ShaderProgram& program, Renderer& renderer);
        };
};
#endif
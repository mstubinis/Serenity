#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_SYSTEM_H
#define ENGINE_RENDERER_PARTICLE_SYSTEM_H

struct Entity;
class  ParticleEmissionProperties;
class  Scene;
class  Camera;
class  ShaderProgram;
class  Material;
class  Viewport;
namespace Engine::priv {
    class Renderer;
};

#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>

constexpr unsigned int MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME = 12U;

namespace Engine::priv {
    class ParticleSystem final : public Engine::NonCopyable, public Engine::NonMoveable {
        friend class ::ParticleEmitter;
        friend class ::Particle;
        private:
            std::vector<ParticleEmitter>           m_ParticleEmitters;
            std::vector<Particle>                  m_Particles;

            std::stack<size_t>                     m_ParticleEmitterFreelist;
            std::stack<size_t>                     m_ParticleFreelist;
            std::mutex                             m_Mutex;

            void internal_update_emitters(const float dt);
            void internal_update_particles(const float dt, Camera& camera);
        public:
            struct ParticleDOD final {
                glm::vec4  PositionAndScaleX;
                glm::vec2  ScaleYAndAngle;
                glm::uvec2 MatIDAndPackedColor;

                ParticleDOD() = delete;
                ParticleDOD(glm::vec4&& posAndScaleX, glm::vec2&& scaleYAndAngle, glm::uvec2&& matIDAndPackedColor) 
                    : PositionAndScaleX(std::move(posAndScaleX))
                    , ScaleYAndAngle(std::move(scaleYAndAngle)) 
                    , MatIDAndPackedColor(std::move(matIDAndPackedColor)) 
                {}
                ParticleDOD(const ParticleDOD& other)                = delete;
                ParticleDOD& operator=(const ParticleDOD& other)     = delete;
                ParticleDOD(ParticleDOD&& other) noexcept            = default;
                ParticleDOD& operator=(ParticleDOD&& other) noexcept = default;
                ~ParticleDOD() = default;
            };

            std::vector<ParticleDOD>                                 ParticlesDOD;
            std::unordered_map<Material*, unsigned int>              MaterialToIndex;
            std::unordered_map<unsigned int, Material*>              MaterialToIndexReverse;
            std::unordered_map<unsigned int, unsigned int>           MaterialIDToIndex;

            //for the threads...
            std::vector<std::vector<ParticleDOD>>                    THREAD_PART_1;
            std::vector<std::unordered_map<Material*, unsigned int>> THREAD_PART_4;
            std::vector<std::unordered_map<unsigned int, Material*>> THREAD_PART_5;
        public:
            ParticleSystem(unsigned int maxEmitters, unsigned int maxParticles);

            ParticleEmitter* add_emitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent);

            bool add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);
            bool add_particle(ParticleEmitter& emitter);

            inline CONSTEXPR std::vector<ParticleEmitter>& getParticleEmitters() noexcept { return m_ParticleEmitters; }
            inline CONSTEXPR std::vector<Particle>& getParticles() noexcept { return m_Particles; }

            void update(const float dt, Camera& camera);
            void render(Viewport& viewport, Camera& camera, ShaderProgram& program, Renderer& renderer);
        };
};
#endif
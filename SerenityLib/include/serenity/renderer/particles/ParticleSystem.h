#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_SYSTEM_H
#define ENGINE_RENDERER_PARTICLE_SYSTEM_H

class  Entity;
class  ParticleEmissionProperties;
class  Scene;
class  Camera;
class  ShaderProgram;
class  Material;
class  Viewport;
namespace Engine::priv {
    class RenderModule;
};

#include <serenity/renderer/particles/Particle.h>
#include <serenity/renderer/particles/ParticleEmitter.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/containers/PartialVectorPOD.h>
#include <serenity/resources/Handle.h>
#include <vector>
#include <queue>
#include <mutex>
#include <serenity/system/TypeDefs.h>

constexpr uint32_t MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME = 12U;

namespace Engine::priv {
    class ParticleSystem final {
        friend class ::ParticleEmitter;
        friend class ::Particle;
        private:
            std::vector<ParticleEmitter>    m_ParticleEmitters;
            std::vector<Particle>           m_Particles;

            std::queue<size_t>              m_ParticleEmitterFreelist;
            std::queue<size_t>              m_ParticleFreelist;
            mutable std::mutex              m_Mutex;

            void internal_update_emitters(const float dt);
            void internal_update_particles(const float dt, Camera& camera);
        public:

#define ENGINE_PARTICLES_HALF_SIZE //use this to reduce particle data size at the cost of precision. this small size uses 16 bytes per particle

#if defined(ENGINE_PARTICLES_HALF_SIZE)
            using ParticleFloatType = uint16_t;
            using ParticleIDType    = uint16_t;
#else
            using ParticleFloatType = float;
            using ParticleIDType    = uint32_t;
#endif

            struct ParticleDOD final {
                ParticleFloatType PositionX;
                ParticleFloatType PositionY;
                ParticleFloatType PositionZ;
                ParticleFloatType ScaleX;

                ParticleFloatType ScaleY;
                ParticleFloatType Angle;

                ParticleIDType    MatID;
                ParticleIDType    PackedColor;

                ParticleDOD() = default;
                ParticleDOD(float X, float Y, float Z, float ScaleX_, float ScaleY_, float Angle_, ParticleIDType MatID_, ParticleIDType PackedColor_)
#if defined(ENGINE_PARTICLES_HALF_SIZE)
                    : PositionX{ Engine::Math::Float16From32(X) }
                    , PositionY{ Engine::Math::Float16From32(Y) }
                    , PositionZ{ Engine::Math::Float16From32(Z) }
                    , ScaleX{ Engine::Math::Float16From32(ScaleX_) }
                    , ScaleY{ Engine::Math::Float16From32(ScaleY_) }
                    , Angle{ Engine::Math::Float16From32(Angle_) }
#else
                    : PositionX{ X }
                    , PositionY{ Y }
                    , PositionZ{ Z }
                    , ScaleX{ ScaleX_ }
                    , ScaleY{ ScaleY_ }
                    , Angle{ Angle_ }
#endif
                    , MatID{ MatID_ }
                    , PackedColor{ PackedColor_ }
                {}
                ParticleDOD(const ParticleDOD& other)                = delete;
                ParticleDOD& operator=(const ParticleDOD& other)     = delete;
                ParticleDOD(ParticleDOD&& other) noexcept            = default;
                ParticleDOD& operator=(ParticleDOD&& other) noexcept = default;
                ~ParticleDOD() = default;
            };

            Engine::partial_vector_pod<ParticleDOD>                  ParticlesDOD;

            std::unordered_map<Material*, uint32_t>              MaterialToIndex;
            std::unordered_map<uint32_t, Material*>              MaterialToIndexReverse;
            std::unordered_map<uint32_t, uint32_t>           MaterialIDToIndex;

            //for the threads...
            std::vector<std::vector<ParticleDOD>>                    THREAD_PART_1;
            std::vector<std::unordered_map<Material*, uint32_t>> THREAD_PART_4;
            std::vector<std::unordered_map<uint32_t, Material*>> THREAD_PART_5;
        public:
            ParticleSystem(uint32_t maxEmitters, uint32_t maxParticles);
            ParticleSystem(const ParticleSystem&)                = delete;
            ParticleSystem& operator=(const ParticleSystem&)     = delete;
            ParticleSystem(ParticleSystem&&) noexcept            = delete;
            ParticleSystem& operator=(ParticleSystem&&) noexcept = delete;

            ParticleEmitter* add_emitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent);

            bool add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);
            bool add_particle(ParticleEmitter& emitter);

            inline constexpr std::vector<ParticleEmitter>& getParticleEmitters() noexcept { return m_ParticleEmitters; }
            inline constexpr std::vector<Particle>& getParticles() noexcept { return m_Particles; }

            void update(const float dt, Camera& camera);
            void render(Viewport& viewport, Camera& camera, Handle program, RenderModule& renderer);
        };
};

#endif
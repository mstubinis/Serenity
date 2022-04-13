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

#include <serenity/renderer/particles/ParticleIncludes.h>
#include <serenity/renderer/particles/Particle.h>
#include <serenity/renderer/particles/ParticleEmitter.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/containers/PartialVectorPOD.h>
#include <serenity/containers/BimapUnordered.h>
#include <serenity/resources/Handle.h>
#include <vector>
#include <queue>
#include <mutex>
#include <serenity/system/TypeDefs.h>

constexpr uint32_t MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME = 12U;

namespace Engine::priv {
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
            : PositionX{ Engine::Math::PackFloat<ParticleIDType>(X) }
            , PositionY{ Engine::Math::PackFloat<ParticleIDType>(Y) }
            , PositionZ{ Engine::Math::PackFloat<ParticleIDType>(Z) }
            , ScaleX{ Engine::Math::PackFloat<ParticleIDType>(ScaleX_) }
            , ScaleY{ Engine::Math::PackFloat<ParticleIDType>(ScaleY_) }
            , Angle{ Engine::Math::PackFloat<ParticleIDType>(Angle_) }
            , MatID{ MatID_ }
            , PackedColor{ PackedColor_ }
        {}
        ParticleDOD(const ParticleDOD&)                = delete;
        ParticleDOD& operator=(const ParticleDOD&)     = delete;
        ParticleDOD(ParticleDOD&&) noexcept            = default;
        ParticleDOD& operator=(ParticleDOD&&) noexcept = default;
    };
}


namespace Engine::priv {
    class ParticleSystem final {
        friend class ::ParticleEmitter;
        friend class ::Particle;
        private:
            std::vector<ParticleEmitter>    m_ParticleEmitters;
            std::vector<Particle>           m_Particles;

            std::vector<size_t>             m_ParticleEmitterFreelist;
            std::vector<size_t>             m_ParticleFreelist;
            mutable std::mutex              m_Mutex;

            void internal_update_emitters(const float dt);
            void internal_update_particles(const float dt, Camera&);
        public:
            Engine::partial_vector_pod<Engine::priv::ParticleDOD>      ParticlesDOD;

            Engine::unordered_bimap<Material*, uint32_t>               Bimap;
            std::unordered_map<uint32_t, uint32_t>                     MaterialIDToIndex;

            //for the threads...
            std::vector<std::vector<Engine::priv::ParticleDOD>>        THREAD_PART_1;
            std::vector<Engine::unordered_bimap<Material*, uint32_t>>  THREAD_PART_4;
        public:
            ParticleSystem(uint32_t maxEmitters, uint32_t maxParticles);
            ParticleSystem(const ParticleSystem&)                = delete;
            ParticleSystem& operator=(const ParticleSystem&)     = delete;
            ParticleSystem(ParticleSystem&&) noexcept            = delete;
            ParticleSystem& operator=(ParticleSystem&&) noexcept = delete;

            Engine::view_ptr<ParticleEmitter> add_emitter(ParticleEmissionProperties&, Scene&, float lifetime, Entity parent);

            bool add_particle(ParticleEmitter&, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);
            bool add_particle(ParticleEmitter&);

            inline std::vector<ParticleEmitter>& getParticleEmitters() noexcept { return m_ParticleEmitters; }
            inline std::vector<Particle>& getParticles() noexcept { return m_Particles; }

            void update(const float dt, Camera&);
            void render(Viewport&, Camera&, Handle program, RenderModule&);
        };
};

#endif
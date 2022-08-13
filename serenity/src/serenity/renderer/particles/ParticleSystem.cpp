
#include <serenity/renderer/particles/ParticleSystem.h>
#include <serenity/renderer/particles/ParticleEmitter.h>
#include <serenity/renderer/particles/Particle.h>
#include <serenity/renderer/particles/ParticleEmissionProperties.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/renderer/culling/SphereIntersectTest.h>

#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/material/Material.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/system/Engine.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/system/Engine.h>
#include <execution>


#include <serenity/utils/BlockProfiler.h>
#include <serenity/renderer/opengl/APIStateOpenGL.h>

namespace {
    constexpr const float PLANE_RADIUS = 0.5f;

    void internal_update_emitters(const float dt, auto& emitters, Engine::priv::ParticleSystem& system) {
        if (!emitters.empty()) {
            auto lamda_update_emitter = [&system, dt](size_t j, int32_t jobIndex) {
                ParticleEmitter& emitter = system.getParticleEmitters()[j];
                if (emitter.isActive()) {
                    emitter.update(j, dt, system, true);
                }
            };
            Engine::priv::threading::addJobSplitVectored(lamda_update_emitter, true, false, emitters.size(), 0);
        }
    }
    void internal_cull_particles(auto& particles, const Camera& camera, auto& part2, auto& part1) {
        const glm::vec3 camPos{ camera.getPosition() };
        auto lamda_culler_particle = [&](size_t j, int32_t jobIndex) {
            Particle& particle     = particles[j];
            const float radius     = PLANE_RADIUS * Engine::Math::Max(particle.m_Scale.x, particle.m_Scale.y);
            const glm::vec3& pos   = particle.position();
            const int sphereTest   = Engine::priv::Culling::sphereIntersectTest(pos, radius, camera);
            const float comparison = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
            if (particle.isActive() && (glm::distance2(pos, camPos) <= (comparison * comparison)) && sphereTest > 0) {
                //its just pretty expensive in general...
                auto itr = part2[jobIndex].find_key(particle.m_Material);
                if (itr == part2[jobIndex].end()) {
                    itr = part2[jobIndex].emplace(particle.m_Material, particle.m_Material->getID()).first;
                }
                ///////////////////////////////////////////
                part1[jobIndex].emplace_back(pos.x - camPos.x, pos.y - camPos.y, pos.z - camPos.z,
                    particle.m_Scale.x, particle.m_Scale.y,
                    particle.m_Angle,
                    ParticleIDType(itr->second),
                    particle.m_Color.toPacked<ParticlePackedColorType>()
                );
            }
        };
        Engine::priv::threading::addJobSplitVectored(lamda_culler_particle, true, false, particles.size(), 0);
    }
}


Engine::priv::ParticleDOD::ParticleDOD(float X, float Y, float Z, float ScaleX_, float ScaleY_, float Angle_, ParticleMaterialIDType MatID_, ParticlePackedColorType PackedColor_)
    : Position{ Engine::Math::PackFloat<ParticleFloatType>(X), Engine::Math::PackFloat<ParticleFloatType>(Y), Engine::Math::PackFloat<ParticleFloatType>(Z) }
    , Scale{ Engine::Math::PackFloat<ParticleFloatType>(ScaleX_), Engine::Math::PackFloat<ParticleFloatType>(ScaleY_) }
    , Angle{ Engine::Math::PackFloat<ParticleFloatType>(Angle_) }
    , MatID{ MatID_ }
    , PackedColor{ PackedColor_ }
{}




Engine::priv::ParticleSystem::ParticleSystem(uint32_t maxEmitters, uint32_t maxParticles) {
    m_ParticleEmitters.reserve(maxEmitters);
    m_Particles.reserve(maxParticles);
    ParticlesDOD.resize(std::min(maxParticles, 300U));

    const auto num_cores        = Engine::hardware_concurrency();
    const auto maxMaterialSlots = std::min(Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().MAX_TEXTURE_IMAGE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME);

    Bimap.reserve(maxMaterialSlots);

    MaterialIDToIndex.reserve(maxMaterialSlots);

    THREAD_PART_PARTICLE_DODS.resize(num_cores);
    THREAD_PART_PARTICLE_MATERIAL_MAPPING.resize(num_cores);

    for (auto& _2 : THREAD_PART_PARTICLE_MATERIAL_MAPPING) {
        _2.reserve(maxMaterialSlots);
    }
}
void Engine::priv::ParticleSystem::internal_update_particles(const float dt, Camera& camera) {
    if (!m_Particles.empty()) {
        auto lamda_update_particle = [this, dt](size_t j, int32_t jobIndex) {
            Particle& particle = m_Particles[j];
            if (particle.m_Timer > 0.0f) {
                auto& prop = *Engine::Resources::getParticleEmissionProperties(particle.m_EmitterSource->m_Properties);
                particle.m_Timer           += dt;
                particle.m_Scale           += prop.m_ChangeInScaleFunctor(dt);
                particle.m_Color            = prop.m_ColorFunctor(dt, prop.getLifetime(), particle.m_Timer);
                particle.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(dt);
                particle.m_Angle           += particle.m_AngularVelocity;
                particle.m_Velocity        += prop.m_ChangeInVelocityFunctor(dt);
                particle.m_Position        += (particle.m_Velocity * dt);
                if (particle.m_Timer >= prop.m_Lifetime) {
                    particle.m_Timer = 0.0f;
                    {
                        std::scoped_lock lock{ m_Mutex };
                        m_ParticleFreelist.emplace_back(j);
                    }
                }
            }
        };
        Engine::priv::threading::addJobSplitVectored(lamda_update_particle, true, false, m_Particles.size(), 0);
    }
}

Engine::view_ptr<ParticleEmitter> Engine::priv::ParticleSystem::add_emitter(ParticleEmissionPropertiesHandle properties, Scene& scene, float lifetime, Entity parent) {
    while (!m_ParticleEmitterFreelist.empty()) { //first, try to reuse an empty
        const size_t freeindex = m_ParticleEmitterFreelist.back();
        m_ParticleEmitterFreelist.pop_back();
        if (freeindex >= m_ParticleEmitters.size() /* && !m_ParticleEmitterFreelist.empty()*/) {
            continue;
        }
        m_ParticleEmitters[freeindex].init(scene, properties, lifetime, parent);
        m_ParticleEmitters[freeindex].activate();
        return &m_ParticleEmitters[freeindex];
    }
    if (m_ParticleEmitters.size() < m_ParticleEmitters.capacity()) {
        auto& emitter = m_ParticleEmitters.emplace_back(scene, properties, lifetime, parent);
        emitter.activate();
        return &emitter;
    }
    return nullptr;
}
bool Engine::priv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    while (!m_ParticleFreelist.empty()) { //first, try to reuse an empty
        const size_t freeindex = m_ParticleFreelist.back();
        m_ParticleFreelist.pop_back();
        if (freeindex >= m_Particles.size() /* && !m_ParticleFreelist.empty()*/) {
            continue;
        }
        m_Particles[freeindex].init(emitterPosition, emitterRotation, emitter);
        return true;
    }
    //otherwise, just add it
    if (m_Particles.size() < m_Particles.capacity()) {
        m_Particles.emplace_back(emitterPosition, emitterRotation, emitter);
        return true;
    }
    return false;
}
bool Engine::priv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    auto transform = emitter.getComponent<ComponentTransform>();
    return transform ? add_particle(emitter, transform->getPosition(), transform->getRotation()) : false;
}

void Engine::priv::ParticleSystem::update(const float dt, Camera& camera) {
    internal_update_emitters(dt, m_ParticleEmitters, *this);
    internal_update_particles(dt, camera);
}

void Engine::priv::ParticleSystem::render(Viewport& viewport, Camera& camera, RenderModule& renderModule) {
    const auto particles_size = m_Particles.size();
    if (particles_size == 0 || !viewport.getRenderFlags().has(ViewportRenderingFlag::Particles)) {
        return;
    }
    //now cull, sort, and populate their render lists
    ParticlesDOD.clear();

    Bimap.clear();

    MaterialIDToIndex.clear();

    if (ParticlesDOD.capacity() < particles_size) {
        ParticlesDOD.resize(particles_size);
    }

    const auto reserve_size = particles_size / Engine::hardware_concurrency();

    for (auto& particleDODVector : THREAD_PART_PARTICLE_DODS)
        particleDODVector.clear();
    for (auto& mappingPart : THREAD_PART_PARTICLE_MATERIAL_MAPPING) 
        mappingPart.clear();
    for (auto& particleDODVector : THREAD_PART_PARTICLE_DODS) 
        particleDODVector.reserve(reserve_size);

    internal_cull_particles(m_Particles, camera, THREAD_PART_PARTICLE_MATERIAL_MAPPING, THREAD_PART_PARTICLE_DODS);

    //merge the thread collections into the main collections
    for (auto& particleDODVector : THREAD_PART_PARTICLE_DODS) {
        for (auto& particleDOD : particleDODVector) {
            ParticlesDOD.push(std::move(particleDOD));
        }
    }
    for (auto& mappingPart : THREAD_PART_PARTICLE_MATERIAL_MAPPING) {
        Bimap.merge(mappingPart);
    }
    //sorting
    auto lambda = [](const ParticleDOD& l, const ParticleDOD& r) {
        return glm::length2(Engine::Math::ToFloatVec3(l.Position)) > glm::length2(Engine::Math::ToFloatVec3(r.Position));
    };
    Engine::sort(std::execution::par_unseq, ParticlesDOD, lambda);

    renderModule.m_Pipeline->renderParticles(*this, camera);
}

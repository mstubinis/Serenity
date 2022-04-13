
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

Engine::priv::ParticleSystem::ParticleSystem(uint32_t maxEmitters, uint32_t maxParticles) {
    m_ParticleEmitters.reserve(maxEmitters);
    m_Particles.reserve(maxParticles);
    ParticlesDOD.resize(std::min(maxParticles, 300U));

    const auto num_cores        = Engine::hardware_concurrency();
    const auto maxMaterialSlots = std::min(Engine::priv::OpenGLState::constants.MAX_TEXTURE_IMAGE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME);

    Bimap.reserve(maxMaterialSlots);

    MaterialIDToIndex.reserve(maxMaterialSlots);

    THREAD_PART_1.resize(num_cores);
    THREAD_PART_4.resize(num_cores);

    for (auto& _4 : THREAD_PART_4) {
        _4.reserve(maxMaterialSlots);
    }
}

void Engine::priv::ParticleSystem::internal_update_emitters(const float dt) {
    if (!m_ParticleEmitters.empty()) {
        auto lamda_update_emitter = [this, dt](ParticleEmitter& emitter, size_t j, size_t k) {
            if (emitter.isActive()) {
                emitter.update(j, dt, *this, true);
            }
        };
        Engine::priv::threading::addJobSplitVectored(lamda_update_emitter, m_ParticleEmitters, true);
    }  
}

void Engine::priv::ParticleSystem::internal_update_particles(const float dt, Camera& camera) {
    if (!m_Particles.empty()) {
        auto lamda_update_particle = [this, dt](Particle& particle, size_t j, size_t k) {
            if (particle.m_Timer > 0.0f) {
                auto& prop = *particle.m_EmitterSource->m_Properties;
                particle.m_Timer           += dt;
                particle.m_Scale           += prop.m_ChangeInScaleFunctor(dt, particle);
                particle.m_Color            = prop.m_ColorFunctor(dt, particle);
                particle.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(dt, particle);
                particle.m_Angle           += particle.m_AngularVelocity;
                particle.m_Velocity        += prop.m_ChangeInVelocityFunctor(dt, particle);
                particle.m_Position        += (particle.m_Velocity * dt);
                if (particle.m_Timer >= prop.m_Lifetime) {
                    particle.m_Timer = 0.0f;
                    {
                        std::lock_guard lock{ m_Mutex };
                        m_ParticleFreelist.emplace_back(j);
                    }
                }
            }
        };
        Engine::priv::threading::addJobSplitVectored(lamda_update_particle, m_Particles, true);
    }
}

Engine::view_ptr<ParticleEmitter> Engine::priv::ParticleSystem::add_emitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    while (!m_ParticleEmitterFreelist.empty()) { //first, try to reuse an empty
        size_t freeindex = m_ParticleEmitterFreelist.back();
        m_ParticleEmitterFreelist.pop_back();
        if (freeindex >= m_ParticleEmitters.size() && !m_ParticleEmitterFreelist.empty()) {
            continue;
        }
        //if (!m_ParticleEmitters[freeindex].isActive()) {
            m_ParticleEmitters[freeindex].init(properties, scene, lifetime, parent);
            m_ParticleEmitters[freeindex].activate();
            return &m_ParticleEmitters[freeindex];
        //}
    }
    if (m_ParticleEmitters.size() < m_ParticleEmitters.capacity()) {
        auto& emitter = m_ParticleEmitters.emplace_back(properties, scene, lifetime, parent);
        emitter.activate();
        return &emitter;
    }
    return nullptr;
}
bool Engine::priv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    while (!m_ParticleFreelist.empty()) { //first, try to reuse an empty
        size_t freeindex = m_ParticleFreelist.back();
        m_ParticleFreelist.pop_back();
        if (freeindex >= m_Particles.size() && !m_ParticleFreelist.empty()) {
            continue;
        }
        //if (!m_Particles[freeindex].isActive()) {
            m_Particles[freeindex].init(emitterPosition, emitterRotation, emitter);
            return true;
        //}
    }
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
    internal_update_emitters(dt);
    internal_update_particles(dt, camera);
}

void Engine::priv::ParticleSystem::render(Viewport& viewport, Camera& camera, Handle program, RenderModule& renderModule) {
    const auto particles_size = m_Particles.size();
    if (particles_size == 0 || !viewport.getRenderFlags().has(ViewportRenderingFlag::Particles)) {
        return;
    }
    //now cull, sort, and populate their render lists
    auto& planeMesh = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().get<Mesh>();
    ParticlesDOD.clear();

    Bimap.clear();

    MaterialIDToIndex.clear();

    if (ParticlesDOD.capacity() < particles_size) {
        ParticlesDOD.resize(particles_size);
    }

    const auto reserve_size = particles_size / Engine::hardware_concurrency();

    for (auto& _1 : THREAD_PART_1) _1.clear();
    for (auto& _4 : THREAD_PART_4) _4.clear();
    for (auto& _1 : THREAD_PART_1) _1.reserve(reserve_size);

    glm::vec3 camPos           = glm::vec3{ camera.getPosition() };
    auto lamda_culler_particle = [&](Particle& particle, size_t j, size_t k) {
        float radius           = planeMesh.getRadius() * Math::Max(particle.m_Scale.x, particle.m_Scale.y);
        const glm::vec3& pos   = particle.position();
        const int sphereTest   = Engine::priv::Culling::sphereIntersectTest(pos, radius, camera);
        float comparison       = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
        if (particle.isActive() && (glm::distance2(pos, camPos) <= (comparison * comparison)) && sphereTest > 0) {
            //its just pretty expensive in general...
            if (!THREAD_PART_4[k].contains_key(particle.m_Material)) {
                THREAD_PART_4[k].emplace(particle.m_Material, particle.m_Material->getID());
            }
            ///////////////////////////////////////////
            THREAD_PART_1[k].emplace_back(pos.x - camPos.x, pos.y - camPos.y, pos.z - camPos.z,
                particle.m_Scale.x, particle.m_Scale.y, 
                particle.m_Angle,
                ParticleIDType(THREAD_PART_4[k].at(particle.m_Material)), 
                particle.m_Color.toPacked<ParticlePackedColorType>()
            );
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_culler_particle, m_Particles, true);

    //merge the thread collections into the main collections
    for (auto& _1 : THREAD_PART_1) { 
        for (auto& particleDOD : _1) {
            ParticlesDOD.push(std::move(particleDOD));
        }
    }
    for (auto& _4 : THREAD_PART_4) { 
        Bimap.merge(_4);
    }
    //sorting
    auto lambda = [](const ParticleDOD& l, const ParticleDOD& r) {
        glm::vec3 lPos{ Math::ToFloat(l.PositionX), Math::ToFloat(l.PositionY), Math::ToFloat(l.PositionZ) };
        glm::vec3 rPos{ Math::ToFloat(r.PositionX), Math::ToFloat(r.PositionY), Math::ToFloat(r.PositionZ) };
        return glm::length2(lPos) > glm::length2(rPos);
    };
    Engine::sort(std::execution::par_unseq, ParticlesDOD, lambda);

    renderModule.m_Pipeline->renderParticles(*this, camera, program);
}

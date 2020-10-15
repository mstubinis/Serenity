#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/system/Engine.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/system/Engine.h>

Engine::priv::ParticleSystem::ParticleSystem(unsigned int maxEmitters, unsigned int maxParticles) {
    m_ParticleEmitters.reserve(maxEmitters);
    m_Particles.reserve(maxParticles);
    ParticlesDOD.resize(std::min(maxParticles, 300U));

    const auto maxMaterialSlots = std::min(Engine::priv::OpenGLState::MAX_TEXTURE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME);

    MaterialToIndex.reserve(maxMaterialSlots);
    MaterialToIndexReverse.reserve(maxMaterialSlots);
    MaterialIDToIndex.reserve(maxMaterialSlots);

    const auto num_cores = Engine::hardware_concurrency();
    THREAD_PART_1.resize(num_cores);

    THREAD_PART_4.resize(num_cores);
    THREAD_PART_5.resize(num_cores);
    for (auto& _4 : THREAD_PART_4) {
        _4.reserve(maxMaterialSlots);
    }
    for (auto& _5 : THREAD_PART_5) {
        _5.reserve(maxMaterialSlots);
    }
}

void Engine::priv::ParticleSystem::internal_update_emitters(const float dt) {
    if (m_ParticleEmitters.size() == 0) {
        return;
    }  
    auto lamda_update_emitter = [this, dt](ParticleEmitter& emitter, size_t j, size_t k) {
        if (emitter.isActive()) {
            emitter.update(j, dt, *this, true);
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_update_emitter, m_ParticleEmitters, true, 0U);
}

void Engine::priv::ParticleSystem::internal_update_particles(const float dt, Camera& camera) {
    if (m_Particles.size() == 0) {
        return;
    }
    auto lamda_update_particle = [this, dt](Particle& particle, size_t j, size_t k) {
        if (particle.m_Timer > 0.0f) {
            auto& prop                  = *particle.m_EmitterSource->m_Properties;
            particle.m_Timer           += dt;
            particle.m_Scale           += prop.m_ChangeInScaleFunctor(dt, particle);
            particle.m_Color            = prop.m_ColorFunctor(dt, particle);
            particle.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(dt, particle);
            particle.m_Angle           += particle.m_AngularVelocity;
            particle.m_Velocity        += prop.m_ChangeInVelocityFunctor(dt, particle);
            particle.m_Position        += (particle.m_Velocity * dt);
            if (particle.m_Timer >= prop.m_Lifetime) {
                particle.m_Timer  = 0.0f;
                {
                    std::unique_lock lock(m_SharedMutex);
                    m_ParticleFreelist.emplace(j);
                }
            }
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_update_particle, m_Particles, true, 0U);
}

ParticleEmitter* Engine::priv::ParticleSystem::add_emitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    while (m_ParticleEmitterFreelist.size() > 0) { //first, try to reuse an empty
        size_t freeindex = m_ParticleEmitterFreelist.front();
        m_ParticleEmitterFreelist.pop();
        if (freeindex >= m_ParticleEmitters.size() && m_ParticleEmitterFreelist.size() > 0) {
            continue;
        }
        if (!m_ParticleEmitters[freeindex].isActive()) {
            m_ParticleEmitters[freeindex].init(properties, scene, lifetime, parent);
            m_ParticleEmitters[freeindex].activate();
            return &m_ParticleEmitters[freeindex];
        }
    }
    if (m_ParticleEmitters.size() < m_ParticleEmitters.capacity()) {
        auto& emitter = m_ParticleEmitters.emplace_back(properties, scene, lifetime, parent);
        emitter.activate();
        return &emitter;
    }
    return nullptr;
}
bool Engine::priv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    while (m_ParticleFreelist.size() > 0) { //first, try to reuse an empty
        size_t freeindex = m_ParticleFreelist.front();
        m_ParticleFreelist.pop();
        if (freeindex >= m_Particles.size() && m_ParticleFreelist.size() > 0) {
            continue;
        }
        if (!m_Particles[freeindex].isActive()) {
            m_Particles[freeindex].init(emitterPosition, emitterRotation, emitter);
            return true;
        }
    }
    if (m_Particles.size() < m_Particles.capacity()) {
        m_Particles.emplace_back(emitterPosition, emitterRotation, emitter);
        return true;
    }
    return false;
}
bool Engine::priv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    auto body = emitter.getComponent<ComponentBody>();
    return (body) ? add_particle(emitter, body->getPosition(), body->getRotation()) : false;
}

void Engine::priv::ParticleSystem::update(const float dt, Camera& camera) {
    internal_update_emitters(dt);
    internal_update_particles(dt, camera);
}

void Engine::priv::ParticleSystem::render(Viewport& viewport, Camera& camera, Handle program, RenderModule& renderer) {
    const auto particles_size = m_Particles.size();
    if (particles_size == 0 || !viewport.getRenderFlags().has(ViewportRenderingFlag::Particles)) {
        return;
    }

    //now cull, sort, and populate their render lists
    auto& planeMesh = *priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().get<Mesh>();
    ParticlesDOD.clear();

    MaterialToIndex.clear();
    MaterialToIndexReverse.clear();
    MaterialIDToIndex.clear();

    if (ParticlesDOD.capacity() < particles_size) {
        ParticlesDOD.resize(particles_size);
    }
    //auto start = chrono::high_resolution_clock::now();

    const auto reserve_size = particles_size / Engine::hardware_concurrency();

    for (auto& _1 : THREAD_PART_1) _1.clear();
    for (auto& _4 : THREAD_PART_4) _4.clear();
    for (auto& _5 : THREAD_PART_5) _5.clear();

    for (auto& _1 : THREAD_PART_1) _1.reserve(reserve_size);

    glm::vec3 camPos           = glm::vec3(camera.getPosition());
    auto lamda_culler_particle = [&](Particle& particle, size_t j, size_t k) {
        float radius           = planeMesh.getRadius() * Math::Max(particle.m_Scale.x, particle.m_Scale.y);
        const glm::vec3& pos   = particle.position();
        const uint sphereTest  = camera.sphereIntersectTest(pos, radius);
        float comparison       = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
        if (particle.isActive() && (glm::distance2(pos, camPos) <= (comparison * comparison)) && sphereTest > 0) {
            //its just pretty expensive in general...
            if (!THREAD_PART_4[k].contains(particle.m_Material)) {
                THREAD_PART_4[k].try_emplace(particle.m_Material,       particle.m_Material->id());
                THREAD_PART_5[k].try_emplace(particle.m_Material->id(), particle.m_Material);
            }
            ///////////////////////////////////////////

            THREAD_PART_1[k].emplace_back(
                pos.x - camPos.x,
                pos.y - camPos.y,
                pos.z - camPos.z,
                particle.m_Scale.x, 
                particle.m_Scale.y, 
                particle.m_Angle,
                (ParticleSystem::ParticleIDType)THREAD_PART_4[k].at(particle.m_Material), 
#if defined(ENGINE_PARTICLES_HALF_SIZE)
                particle.m_Color.toPackedShort()
#else
                particle.m_Color.toPackedInt()
#endif
            );
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_culler_particle, m_Particles, true, 0);

    //merge the thread collections into the main collections
    for (auto& _1 : THREAD_PART_1) { 
        for (auto& item : _1) {
            ParticlesDOD.push(std::move(item));
        }
        //ParticlesDOD.insert(ParticlesDOD.end(), std::make_move_iterator(_1.begin()), std::make_move_iterator(_1.end())); 
    }
    for (auto& _4 : THREAD_PART_4) { 
        MaterialToIndex.merge(_4); 
    }
    for (auto& _5 : THREAD_PART_5) { 
        MaterialToIndexReverse.merge(_5); 
    }

    //auto z = (chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - start));
    //float f = static_cast<float>(z.count()) / 1000000.0f;
    //Core::m_Engine->m_DebugManager.addDebugLine(to_string(f));

    //sorting
    auto lambda = [&camPos](const ParticleDOD& l, const ParticleDOD& r) {
#if defined(ENGINE_PARTICLES_HALF_SIZE)
        glm::vec3 lPos(
            Engine::Math::Float32From16(l.PositionX), 
            Engine::Math::Float32From16(l.PositionY),
            Engine::Math::Float32From16(l.PositionZ)
        );
        glm::vec3 rPos(
            Engine::Math::Float32From16(r.PositionX),
            Engine::Math::Float32From16(r.PositionY),
            Engine::Math::Float32From16(r.PositionZ)
        );
#else
        glm::vec3 lPos(
            l.PositionX,
            l.PositionY,
            l.PositionZ
        );
        glm::vec3 rPos(
            r.PositionX,
            r.PositionY,
            r.PositionZ
        );
#endif
        return glm::length2(lPos) > glm::length2(rPos);
    };
    std::sort(std::execution::par_unseq, ParticlesDOD.begin(), ParticlesDOD.end(), lambda);

    renderer.m_Pipeline->renderParticles(*this, camera, program);
}

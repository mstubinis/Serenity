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
#include <core/engine/system/Engine.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/system/Engine.h>

using namespace std;
using namespace Engine;

priv::ParticleSystem::ParticleSystem() {
    m_ParticleEmitters.reserve(NUMBER_OF_PARTICLE_EMITTERS_LIMIT);
    m_Particles.reserve(NUMBER_OF_PARTICLE_LIMIT);

    MaterialToIndex.reserve(10);
    MaterialToIndexReverse.reserve(10);
    MaterialIDToIndex.reserve(10);

    auto num_cores = Engine::hardware_concurrency();
    THREAD_PART_1.resize(num_cores);

    THREAD_PART_4.resize(num_cores);
    THREAD_PART_5.resize(num_cores);
    for (auto& _4 : THREAD_PART_4)
        _4.reserve(10);
    for (auto& _5 : THREAD_PART_5)
        _5.reserve(10);
}
priv::ParticleSystem::~ParticleSystem() {

}


void priv::ParticleSystem::internal_update_emitters(const float dt) {
    if (m_ParticleEmitters.size() == 0) {
        return;
    }
    auto lamda_update_emitter = [&](ParticleEmitter& emitter, size_t j, size_t k) {
        if (emitter.isActive()) {
            emitter.update(j, dt, *this, true);
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_update_emitter, m_ParticleEmitters, true, 0U);
}

void priv::ParticleSystem::internal_update_particles(const float dt, Camera& camera) {
    if (m_Particles.size() == 0) {
        return;
    }

    //update individual particles
    auto lamda_update_particle = [&](Particle& particle, size_t j, size_t k) {
        if (particle.m_Timer > 0.0f) {
            particle.m_Timer           += dt;
            auto& prop                  = *particle.m_EmitterSource->m_Properties;

            particle.m_Scale           += prop.m_ChangeInScaleFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);
            particle.m_Color            = prop.m_ColorFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);
            particle.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);
            particle.m_Angle           += particle.m_AngularVelocity;
            particle.m_Velocity        += prop.m_ChangeInVelocityFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);

            particle.m_Position        += (particle.m_Velocity * dt);

            if (particle.m_Timer >= prop.m_Lifetime) {
                particle.m_Timer = 0.0f;
                {
                    std::lock_guard lock(m_Mutex);
                    m_ParticleFreelist.push(j);
                }
            }
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_update_particle, m_Particles, true, 0U);
}

ParticleEmitter* priv::ParticleSystem::add_emitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    if (m_ParticleEmitterFreelist.size() > 0) { //first, try to reuse an empty
        size_t freeindex = m_ParticleEmitterFreelist.top();
        m_ParticleEmitterFreelist.pop();
        if (freeindex >= m_ParticleEmitters.size()) {
            return nullptr;
        }
        m_ParticleEmitters[freeindex].init(properties, scene, lifetime, parent);
        m_ParticleEmitters[freeindex].activate();
        return &m_ParticleEmitters[freeindex];
    }
    if (m_ParticleEmitters.size() < m_ParticleEmitters.capacity()) {
        m_ParticleEmitters.emplace_back(properties, scene, lifetime, parent);
        m_ParticleEmitters.back().activate();
        return &m_ParticleEmitters.back();
    }
    return nullptr;
}
bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    if (m_ParticleFreelist.size() > 0) { //first, try to reuse an empty
        const auto freeindex = m_ParticleFreelist.top();
        m_ParticleFreelist.pop();
        if (freeindex >= m_Particles.size()) {
            return false;
        }
        m_Particles[freeindex].init(emitterPosition, emitterRotation, emitter);
        return true;
    }
    if (m_Particles.size() < m_Particles.capacity()) {
        m_Particles.emplace_back(emitterPosition, emitterRotation, emitter);
        return true;
    }
    return false;
}
bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    const auto& body = *emitter.getComponent<ComponentBody>();
    return add_particle(emitter, body.getPosition(), body.getRotation());
}

void priv::ParticleSystem::update(const float dt, Camera& camera) {
    internal_update_particles(dt, camera);
    internal_update_emitters(dt);
}

void priv::ParticleSystem::render(Viewport& viewport, Camera& camera, ShaderProgram& program, Renderer& renderer) {
    const auto particles_size = m_Particles.size();
    if (particles_size == 0 || !viewport.getRenderFlags().has(ViewportRenderingFlag::Particles)) {
        return;
    }

    //now cull, sort, and populate their render lists
    auto& planeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    ParticlesDOD.clear();

    MaterialToIndex.clear();
    MaterialToIndexReverse.clear();
    MaterialIDToIndex.clear();
    if (ParticlesDOD.capacity() < particles_size) {
        ParticlesDOD.reserve(particles_size);
    }
    //auto start = chrono::high_resolution_clock::now();

    const auto reserve_size = particles_size / Engine::hardware_concurrency();

    for (auto& _1 : THREAD_PART_1) _1.clear();
    for (auto& _4 : THREAD_PART_4) _4.clear();
    for (auto& _5 : THREAD_PART_5) _5.clear();

    for (auto& _1 : THREAD_PART_1) _1.reserve(reserve_size);

    const glm::vec3 camPos     = glm::vec3(camera.getPosition());
    auto lamda_culler_particle = [&](Particle& particle, size_t j, size_t k) {
        const float radius     = planeMesh.getRadius() * Math::Max(particle.m_Scale.x, particle.m_Scale.y);
        const glm::vec3& pos   = particle.position();
        const uint sphereTest  = camera.sphereIntersectTest(pos, radius);
        const float comparison = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
        if (particle.isActive() && (glm::distance2(pos, camPos) <= (comparison * comparison)) && sphereTest > 0) {
            //is is just pretty expensive in general...
            if (!THREAD_PART_4[k].count(particle.m_Material)) {
                THREAD_PART_4[k].try_emplace(particle.m_Material,       particle.m_Material->id());
                THREAD_PART_5[k].try_emplace(particle.m_Material->id(), particle.m_Material);
            }
            ///////////////////////////////////////////

            THREAD_PART_1[k].emplace_back(
                glm::vec4(pos.x, pos.y, pos.z, particle.m_Scale.x),
                glm::vec2(particle.m_Scale.y, particle.m_Angle),
                glm::uvec2(THREAD_PART_4[k].at(particle.m_Material), particle.m_Color.toPackedInt())
            );
        }
    };
    Engine::priv::threading::addJobSplitVectored(lamda_culler_particle, m_Particles, true, 0);

    //merge the thread collections into the main collections
    for (auto& _1 : THREAD_PART_1) { ParticlesDOD.insert(ParticlesDOD.end(), std::make_move_iterator(_1.begin()), std::make_move_iterator(_1.end())); }
    for (auto& _4 : THREAD_PART_4) { MaterialToIndex.merge(_4); }
    for (auto& _5 : THREAD_PART_5) { MaterialToIndexReverse.merge(_5); }

    //auto z = (chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - start));
    //float f = static_cast<float>(z.count()) / 1000000.0f;
    //Core::m_Engine->m_DebugManager.addDebugLine(to_string(f));

    //sorting
    auto lambda = [&](const ParticleDOD& l, const ParticleDOD& r) {
        glm::vec3 position1(l.PositionAndScaleX.x, l.PositionAndScaleX.y, l.PositionAndScaleX.z);
        glm::vec3 position2(r.PositionAndScaleX.x, r.PositionAndScaleX.y, r.PositionAndScaleX.z);
        return glm::distance2(position1, camPos) > glm::distance2(position2, camPos);
    };
    std::sort(std::execution::par_unseq, ParticlesDOD.begin(), ParticlesDOD.end(), lambda);

    renderer.m_Pipeline->renderParticles(*this, camera, program);
}

vector<ParticleEmitter>& priv::ParticleSystem::getParticleEmitters() {
    return m_ParticleEmitters;
}
vector<Particle>& priv::ParticleSystem::getParticles() {
    return m_Particles;
}
stack<size_t>& priv::ParticleSystem::getParticleEmittersFreelist() {
    return m_ParticleEmitterFreelist;
}
stack<size_t>& priv::ParticleSystem::getParticlesFreelist() {
    return m_ParticleFreelist;
}
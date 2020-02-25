#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/system/Engine.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/system/Engine.h>

#include <execution>
#include <glm/gtx/norm.hpp>


using namespace std;
using namespace Engine;

priv::ParticleSystem::ParticleSystem() {
    m_ParticleEmitters.reserve(NUMBER_OF_PARTICLE_EMITTERS_LIMIT);
    m_Particles.reserve(NUMBER_OF_PARTICLE_LIMIT);
}
priv::ParticleSystem::~ParticleSystem() {

}


void priv::ParticleSystem::internal_update_emitters(const float& dt) {
    if (m_ParticleEmitters.size() == 0) {
        return;
    }

    auto lamda_update_emitter = [&](ParticleEmitter& emitter, const size_t& j) {
        emitter.update(j, dt, *this, true);
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_emitter, m_ParticleEmitters, true);
}
void priv::ParticleSystem::internal_update_particles(const float& dt, const Camera& camera) {
    if (m_Particles.size() == 0) {
        return;
    }

    const auto camPos = glm::vec3(camera.getPosition());
    auto lamda_update_particle = [&](Particle& particle, const size_t& j) {
        particle.update(j, dt, *this, camPos, true);
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_particle, m_Particles, true);
}

ParticleEmitter* priv::ParticleSystem::add_emitter(ParticleEmitter& emitter) {
    if (m_ParticleEmitterFreelist.size() > 0) { //first, try to reuse an empty
        const auto freeindex = m_ParticleEmitterFreelist.top();
        m_ParticleEmitterFreelist.pop();
        if (freeindex >= m_ParticleEmitters.size()) {
            return nullptr;
        }
        using std::swap;
        swap(m_ParticleEmitters[freeindex], emitter);
        return &m_ParticleEmitters[freeindex];
    }
    if (m_ParticleEmitters.size() < m_ParticleEmitters.capacity()) {
        m_ParticleEmitters.push_back(std::move(emitter));
        return &m_ParticleEmitters[m_ParticleEmitters.size() - 1];
    }
    return nullptr;
}
const bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    if (m_ParticleFreelist.size() > 0) { //first, try to reuse an empty
        const auto freeindex = m_ParticleFreelist.top();
        m_ParticleFreelist.pop();
        if (freeindex >= m_Particles.size()) {
            return false;
        }
        ParticleData data(*emitter.m_Properties, emitter, m_Particles[freeindex]);
        m_Particles[freeindex].init(std::move(data), emitterPosition, emitterRotation, emitter.m_Parent);
        return true;
    }
    if (m_Particles.size() < m_Particles.capacity()) {
        m_Particles.emplace_back(emitterPosition, emitterRotation, emitter);
        return true;
    }
    return false;
}
const bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    const auto& body = *emitter.getComponent<ComponentBody>();
    return add_particle(emitter, body.position(), body.rotation());
}


void priv::ParticleSystem::update(const float& dt, const Camera& camera) {
    internal_update_particles(dt, camera);
    internal_update_emitters(dt);
}
void priv::ParticleSystem::render(const Camera& camera, ShaderProgram& program, Renderer& renderer) {
    if (m_Particles.size() == 0) {
        return;
    }

    vector<Particle*> seen;
    seen.reserve(m_Particles.size());

    const auto cameraPosition = glm::vec3(camera.getPosition());

    auto& planeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();

    auto lamda_culler_particle = [&](Particle& particle, const size_t& j) {
        const float radius    = planeMesh.getRadius() * Math::Max(particle.m_Data.m_Scale.x, particle.m_Data.m_Scale.y);
        const uint sphereTest = camera.sphereIntersectTest(particle.m_Position, radius); //per mesh instance radius instead?
        float comparison      = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
        if (particle.m_Hidden || sphereTest == 0 || glm::distance2(particle.m_Position, cameraPosition) > comparison * comparison) {
            particle.m_PassedRenderCheck = false;
        }else{
            particle.m_PassedRenderCheck = true;
            std::lock_guard<std::mutex> lock(m_Mutex);
            seen.push_back(&particle);
        }
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_culler_particle, m_Particles, true);


    auto lambda_sorter = [&](const Particle* lhs, const Particle* rhs) {
        return glm::distance2(lhs->m_Position, cameraPosition) > glm::distance2(rhs->m_Position, cameraPosition);
    };
    std::sort(std::execution::par_unseq, seen.begin(), seen.end(), lambda_sorter);

    renderer._bindShaderProgram(&program);

    planeMesh.bind();
    for (auto& particle : seen) {
        //if (particle.m_PassedRenderCheck) { //TODO: using "seen" vector for now, do not need bool check, should profile using seen vector over using bool and full vector...
            renderer.m_Pipeline->renderParticle(*particle);
        //}
    }
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
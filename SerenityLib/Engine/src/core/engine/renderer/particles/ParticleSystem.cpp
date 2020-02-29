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


void priv::ParticleSystem::internal_update_emitters(const float dt) {
    if (m_ParticleEmitters.size() == 0) {
        return;
    }

    auto lamda_update_emitter = [&](ParticleEmitter& emitter, const size_t& j) {
        emitter.update(j, dt, *this, true);
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_emitter, m_ParticleEmitters, true);
}
void priv::ParticleSystem::internal_update_particles(const float dt, const Camera& camera) {
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
        m_Particles[freeindex].init(emitterPosition, emitterRotation, emitter);
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



/*
template<class T1, class T2>
std::vector<T1> sort_by(const std::vector<T1>& vin, const std::vector<T2>& keys, const Camera& camera) {
    std::vector<std::size_t> indices;
    indices.reserve(vin.size());
    for (auto&& unused : keys) {
        indices.push_back(indices.size());
    }

    const glm::vec3 cameraPosition = glm::vec3(camera.getPosition());

    auto lambda = [&](std::size_t l, std::size_t r) {
        return glm::distance2(keys[l], cameraPosition) > glm::distance2(keys[r], cameraPosition);
    };

    std::sort(std::begin(indices), std::end(indices), lambda);
    std::vector<T1> r;
    r.reserve(vin.size());
    for (auto& i : indices) {
        r.push_back(vin[i]);
    }
    return r;
}
*/

void priv::ParticleSystem::update(const float dt, const Camera& camera) {
    internal_update_particles(dt, camera);
    internal_update_emitters(dt);
}
void priv::ParticleSystem::render(const Camera& camera, ShaderProgram& program, Renderer& renderer) {
    if (m_Particles.size() == 0) {
        return;
    }
    renderer.m_Pipeline->renderParticles(*this, camera, program, m_Mutex);
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
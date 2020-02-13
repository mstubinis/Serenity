#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/Particle.h>

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
    if (m_ParticleEmitters.size() == 0)
        return;

    //TODO: determine when the overhead for creating the threading calls will out-perform the single core call for not so many emitters on the screen
    auto lamda_update_emitters = [&](pair<size_t, size_t>& pair_) {
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            m_ParticleEmitters[j].update_multithreaded(j, dt, *this);
        }
    };
    auto split = priv::threading::splitVectorPairs(m_ParticleEmitters);
    for (auto& pair_ : split) {
        priv::Core::m_Engine->m_ThreadManager.add_job_ref_engine_controlled(lamda_update_emitters, pair_);
        //priv::threading::addJobRef(lamda_update_emitters, pair_);
    }
    priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
    //priv::threading::waitForAll();
}
void priv::ParticleSystem::internal_update_particles(const float& dt) {
    if (m_Particles.size() == 0)
        return;
    auto lamda_update_particles = [&](pair<size_t, size_t>& pair_) {
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            m_Particles[j].update_multithreaded(j, dt, *this);
        }
    };
    auto split = priv::threading::splitVectorPairs(m_Particles);
    for (auto& pair_ : split) {
        priv::Core::m_Engine->m_ThreadManager.add_job_ref_engine_controlled(lamda_update_particles, pair_);
        //priv::threading::addJobRef(lamda_update_particles, pair_);
    }
    priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
    //priv::threading::waitForAll();
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
        m_Particles[freeindex].init(data, emitterPosition, emitterRotation, emitter.m_Parent);
        return true;
    }
    if (m_Particles.size() < m_Particles.capacity()) {
        Particle particle(emitterPosition, emitterRotation, emitter);
        m_Particles.push_back(std::move(particle));
        return true;
    }
    return false;
}
const bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    auto& body = *emitter.getComponent<ComponentBody>();
    return add_particle(emitter, body.position(), body.rotation());
}

void priv::ParticleSystem::update(const float& dt) {
    internal_update_particles(dt);
    internal_update_emitters(dt);
}
void priv::ParticleSystem::render(Camera& camera, ShaderProgram& program, GBuffer& gBuffer) {
    if (m_Particles.size() == 0)
        return;

    vector<Particle> seen;
    seen.reserve(m_Particles.size());

    const auto cameraPosition = glm::vec3(camera.getPosition());

    auto& planeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();

    auto lamda_culler = [&](pair<size_t, size_t>& pair_, const glm::vec3& camPos) {
        
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            auto& particle = m_Particles[j];

            const float radius = planeMesh.getRadius() * Math::Max(particle.m_Data.m_Scale.x, particle.m_Data.m_Scale.y);
            const uint sphereTest = camera.sphereIntersectTest(particle.m_Position, radius); //per mesh instance radius instead?
            float comparison = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
            if (particle.m_Hidden || sphereTest == 0 || glm::distance2(particle.m_Position, cameraPosition) > comparison * comparison) {
                particle.m_PassedRenderCheck = false;
            }else{
                particle.m_PassedRenderCheck = true;
                std::lock_guard<std::mutex> lock(m_Mutex);
                seen.push_back(particle);
            }
        }
    };
    auto split = priv::threading::splitVectorPairs(m_Particles);
    for (auto& pair_ : split) {
        priv::Core::m_Engine->m_ThreadManager.add_job_ref_engine_controlled(lamda_culler, pair_, cameraPosition);
        //priv::threading::addJobRef(lamda_culler, pair_, cameraPosition);
    }
    priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
    //priv::threading::waitForAll();

    auto lambda_sorter = [&](Particle& lhs, Particle& rhs, const glm::vec3& camPos) {
        return glm::distance2(lhs.m_Position, camPos) > glm::distance2(rhs.m_Position, camPos);
    };
    std::sort(std::execution::par_unseq, seen.begin(), seen.end(), std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, cameraPosition));

    program.bind();
    planeMesh.bind();
    for (auto& particle : seen) {
        //if (particle.m_PassedRenderCheck) { //TODO: using "seen" vector for now, do not need bool check, should profile using seen vector over using bool and full vector...
            particle.render(gBuffer);
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
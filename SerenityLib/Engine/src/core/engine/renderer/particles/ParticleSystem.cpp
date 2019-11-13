#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/Particle.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/shaders/ShaderProgram.h>

#include <execution>


using namespace std;
using namespace Engine;

epriv::ParticleSystem::ParticleSystem() {
    m_ParticleEmitters.reserve(NUMBER_OF_PARTICLE_EMITTERS_LIMIT);
    m_Particles.reserve(NUMBER_OF_PARTICLE_LIMIT);
}
epriv::ParticleSystem::~ParticleSystem() {

}


void epriv::ParticleSystem::internal_update_emitters(const double& dt) {
    if (m_ParticleEmitters.size() == 0)
        return;

    //TODO: determine when the overhead for creating the threading calls will out-perform the single core call for not so many emitters on the screen
    auto lamda_update_emitters = [&](pair<size_t, size_t>& pair_) {
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            m_ParticleEmitters[j].update_multithreaded(j, dt, *this);
        }
    };
    auto split = epriv::threading::splitVectorPairs(m_ParticleEmitters);
    for (auto& pair_ : split) {
        epriv::threading::addJobRef(lamda_update_emitters, pair_);
    }
    epriv::threading::waitForAll();
}
void epriv::ParticleSystem::internal_update_particles(const double& dt) {
    if (m_Particles.size() == 0)
        return;
    auto lamda_update_particles = [&](pair<size_t, size_t>& pair_) {
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            m_Particles[j].update_multithreaded(j, dt, *this);
        }
    };
    auto split = epriv::threading::splitVectorPairs(m_Particles);
    for (auto& pair_ : split) {
        epriv::threading::addJobRef(lamda_update_particles, pair_);
    }
    epriv::threading::waitForAll();
}

ParticleEmitter* epriv::ParticleSystem::add_emitter(ParticleEmitter& emitter) {
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
const bool epriv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    if (m_ParticleFreelist.size() > 0) { //first, try to reuse an empty
        ParticleData data(*emitter.m_Properties);
        const auto freeindex = m_ParticleFreelist.top();
        m_ParticleFreelist.pop();
        if (freeindex >= m_Particles.size()) {
            return false;
        }
        m_Particles[freeindex].init(data, emitterPosition, emitterRotation, emitter.m_Parent);
        return true;
    }
    if (m_Particles.size() < m_Particles.capacity()) {
        Particle particle(emitterPosition, emitterRotation, *emitter.m_Properties, emitter.entity().scene(), emitter.m_Parent);
        m_Particles.push_back(std::move(particle));
        return true;
    }
    return false;
}
const bool epriv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    auto& body = *emitter.getComponent<ComponentBody>();
    return add_particle(emitter, body.position(), body.rotation());
}

void epriv::ParticleSystem::update(const double& dt) {
    internal_update_particles(dt);
    internal_update_emitters(dt);
}
void epriv::ParticleSystem::render(Camera& camera, ShaderProgram& program, GBuffer& gBuffer) {
    if (m_Particles.size() == 0)
        return;

    vector<Particle> seen;
    seen.reserve(m_Particles.size());

    auto lamda_culler = [&](pair<size_t, size_t>& pair_) {
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            auto& particle = m_Particles[j];
            const float radius = Mesh::Plane->getRadius() * Math::Max(particle.m_Data.m_Scale.x, particle.m_Data.m_Scale.y);
            const uint sphereTest = camera.sphereIntersectTest(particle.m_Position, radius); //per mesh instance radius instead?
            decimal comparison = static_cast<decimal>(radius) * static_cast<decimal>(1100.0);
            if (particle.m_Hidden || sphereTest == 0 || camera.getDistanceSquared(particle.m_Position) > comparison * comparison) {
                particle.m_PassedRenderCheck = false;
            }else{
                particle.m_PassedRenderCheck = true;
                m_Mutex.lock();
                seen.push_back(particle);
                m_Mutex.unlock();
            }
        }
    };
    auto lambda_sorter = [&](Particle& lhs, Particle& rhs) {
        return camera.getDistanceSquared(lhs.m_Position) > camera.getDistanceSquared(rhs.m_Position);
    };

    auto split = epriv::threading::splitVectorPairs(m_Particles);
    for (auto& pair_ : split) {
        epriv::threading::addJobRef(lamda_culler, pair_);
    }
    epriv::threading::waitForAll();

    std::sort(std::execution::par_unseq, seen.begin(), seen.end(), lambda_sorter);

    program.bind();
    Mesh::Plane->bind();
    for (auto& particle : seen) {
        //if (particle.m_PassedRenderCheck) { //TODO: using "seen" vector for now, do not need bool check, should profile using seen vector over using bool and full vector...
        particle.render(gBuffer);
        //}
    }
}

vector<ParticleEmitter>& epriv::ParticleSystem::getParticleEmitters() {
    return m_ParticleEmitters;
}
vector<Particle>& epriv::ParticleSystem::getParticles() {
    return m_Particles;
}
stack<size_t>& epriv::ParticleSystem::getParticleEmittersFreelist() {
    return m_ParticleEmitterFreelist;
}
stack<size_t>& epriv::ParticleSystem::getParticlesFreelist() {
    return m_ParticleFreelist;
}
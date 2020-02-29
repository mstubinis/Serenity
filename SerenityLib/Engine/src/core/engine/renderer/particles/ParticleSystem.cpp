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


template<class T1, class T2, class T3>
void sort_by(vector<T1>& vin1, vector<T2>& vin2, vector<T3>& keys, const Camera& camera) {
    const glm::vec3 cameraPosition = glm::vec3(camera.getPosition());
    vector<std::size_t> indices;
    indices.reserve(vin1.size());

    for (auto&& unused : keys) {
        indices.push_back(indices.size());
    }
    auto lambda = [&](std::size_t l, std::size_t r) {
        const glm::vec3 position1 = glm::vec3(keys[l].x, keys[l].y, keys[l].z);
        const glm::vec3 position2 = glm::vec3(keys[r].x, keys[r].y, keys[r].z);
        return glm::distance2(position1, cameraPosition) > glm::distance2(position2, cameraPosition);
    };
    std::sort(std::execution::par_unseq, std::begin(indices), std::end(indices), lambda);


    vector<T1> r1;
    r1.reserve(keys.size());
    for (auto& i : indices) {
        r1.push_back(vin1[i]);
    }

    vector<T2> r2;
    r2.reserve(keys.size());
    for (auto& i : indices) {
        r2.push_back(vin2[i]);
    }

    vector<T3> r3;
    r3.reserve(keys.size());
    for (auto& i : indices) {
        r3.push_back(keys[i]);
    }

    vin1 = r1;
    vin2 = r2;
    keys = r3;
}


void priv::ParticleSystem::internal_update_particles(const float dt, const Camera& camera) {
    if (m_Particles.size() == 0) {
        return;
    }

    //update individual particles
    auto lamda_update_particle = [&](Particle& particle, const size_t& j) {
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
                std::lock_guard lock(m_Mutex);
                m_ParticleFreelist.push(j);
            }
        }
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

void priv::ParticleSystem::update(const float dt, const Camera& camera) {
    internal_update_particles(dt, camera);
    internal_update_emitters(dt);
}
void priv::ParticleSystem::render(const Camera& camera, ShaderProgram& program, Renderer& renderer) {
    if (m_Particles.size() == 0) {
        return;
    }



    //now cull, sort, and populate their render lists
    auto& planeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    PositionAndScaleX.clear();
    ScaleYAndAngle.clear();
    MatIDAndPackedColor.clear();

    MaterialToIndex.clear();
    MaterialToIndex.reserve(10);
    MaterialToIndexReverse.clear();
    MaterialToIndexReverse.reserve(10);

    if (PositionAndScaleX.capacity() < m_Particles.size()) {
        PositionAndScaleX.reserve(m_Particles.size());
        ScaleYAndAngle.reserve(m_Particles.size());
        MatIDAndPackedColor.reserve(m_Particles.size());
    }

    const glm::vec3 camPos = glm::vec3(camera.getPosition());
    auto lamda_culler_particle = [&](Particle& particle, const size_t& j) {
        const float radius = planeMesh.getRadius() * Math::Max(particle.m_Scale.x, particle.m_Scale.y);
        const glm::vec3& pos = particle.position();
        const uint sphereTest = camera.sphereIntersectTest(pos, radius);
        const float comparison = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
        if (particle.isActive() && glm::distance2(pos, camPos) <= comparison * comparison && sphereTest > 0) {
            //particle.m_PassedRenderCheck = true;
            std::lock_guard<std::mutex> lock(m_Mutex);

            if (!MaterialToIndex.count(particle.m_Material)) {
                MaterialToIndex.emplace(particle.m_Material, MaterialToIndex.size());
                MaterialToIndexReverse.emplace(MaterialToIndex.size() - 1U, particle.m_Material);
            }
            PositionAndScaleX.emplace_back(pos.x - camPos.x, pos.y - camPos.y, pos.z - camPos.z, particle.m_Scale.x);
            ScaleYAndAngle.emplace_back(particle.m_Scale.y, particle.m_Angle);
            MatIDAndPackedColor.emplace_back(MaterialToIndex[particle.m_Material], particle.m_Color.toPackedInt());
        }
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_culler_particle, m_Particles, true);

    sort_by(ScaleYAndAngle, MatIDAndPackedColor, PositionAndScaleX, camera);




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
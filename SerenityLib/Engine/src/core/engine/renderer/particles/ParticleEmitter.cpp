#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/system/Engine.h>
#include <core/engine/resources/Engine_Resources.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) 
    : EntityBody{ scene }
{
    addComponent<ComponentBody>();

    /*
    addComponent<ComponentModel>(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), Material::Checkers);
    auto& modelComponent = *getComponent<ComponentModel>();
    modelComponent.getModel().setScale(0.01f, 0.01f, 0.1f);
    modelComponent.getModel().translate(0.0f, 0.0f, 0.1f);
    */
    init(properties, scene, lifetime, parent);
}
void ParticleEmitter::init(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    setProperties(properties);
    removeAllChildren();
    float maxFloat = std::numeric_limits<float>().max();
    setPosition(maxFloat, maxFloat, maxFloat);
    if (!parent.null()) {
        parent.addChild(*this);
    }
    m_Lifetime      = lifetime;
    m_SpawningTimer = m_Properties->m_SpawnRate - 0.01f;

    setLinearVelocity(0.0, 0.0, 0.0);
    setScale(1.0, 1.0, 1.0);
}

ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept 
    : EntityBody{ std::move(other) }
    , m_SpawningTimer{ std::move(other.m_SpawningTimer) }
    , m_Active{ std::move(other.m_Active) }
    , m_Timer{ std::move(other.m_Timer) }
    , m_Lifetime{ std::move(other.m_Lifetime) }
    , m_Parent{ std::move(other.m_Parent) }
    , m_UpdateFunctor{ std::move(other.m_UpdateFunctor) }
    , m_UserData{ std::move(other.m_UserData) }
    , m_Properties{ std::move(other.m_Properties) }
{}
ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& other) noexcept {  
    EntityBody::operator=(std::move(other));
    m_Properties     = std::move(other.m_Properties);
    m_SpawningTimer  = std::move(other.m_SpawningTimer);
    m_Active         = std::move(other.m_Active);
    m_Timer          = std::move(other.m_Timer);
    m_Lifetime       = std::move(other.m_Lifetime);
    m_Parent         = std::move(other.m_Parent);
    m_UpdateFunctor  = std::move(other.m_UpdateFunctor);
    m_UserData       = std::move(other.m_UserData);
    return *this;
}


void ParticleEmitter::setProperties(ParticleEmissionProperties& properties) {
    m_Properties = &properties;
}
void ParticleEmitter::update(size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, bool multi_threaded) {
    //handle spawning
    if (m_Active) {
        m_Timer           += dt;
        m_SpawningTimer   += dt;
        m_UpdateFunctor(this, dt, particleSystem.m_SharedMutex);
        if (m_Properties && m_SpawningTimer > m_Properties->m_SpawnRate) {
            if (multi_threaded) {
                std::unique_lock lock(particleSystem.m_SharedMutex);
                for (uint32_t i = 0; i < m_Properties->m_ParticlesPerSpawn; ++i) {
                    particleSystem.add_particle(*this);
                }
            }else{
                for (uint32_t i = 0; i < m_Properties->m_ParticlesPerSpawn; ++i) {
                    particleSystem.add_particle(*this);
                }
            }
            m_SpawningTimer = 0.0;
        }
        if (m_Lifetime > 0.0 && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer  = 0.0;
            if (multi_threaded) {
                std::unique_lock lock(particleSystem.m_SharedMutex);
                particleSystem.m_ParticleEmitterFreelist.emplace(index);
            }else{
                particleSystem.m_ParticleEmitterFreelist.emplace(index);
            }
        }
    }
}

void ParticleEmitter::applyLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    auto& body = *getComponent<ComponentBody>();
    const auto currVel = body.getLinearVelocity();
    auto newVel = glm_vec3(x, y, z);
    if (local) {
        newVel = body.getRotation() * newVel;
    }
    body.setLinearVelocity(currVel + newVel, false);
}
void ParticleEmitter::applyLinearVelocity(glm_vec3& velocity, const bool local) {
    auto& body = *getComponent<ComponentBody>();
    const auto currVel = body.getLinearVelocity();
    if (local) {
        velocity = body.getRotation() * velocity;
    }
    body.setLinearVelocity(currVel + velocity, false);
}

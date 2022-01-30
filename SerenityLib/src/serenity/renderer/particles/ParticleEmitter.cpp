
#include <serenity/renderer/particles/ParticleSystem.h>
#include <serenity/renderer/particles/ParticleEmitter.h>
#include <serenity/renderer/particles/ParticleEmissionProperties.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>

#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/material/Material.h>

namespace {
    void internal_add_debugging_visual(ParticleEmitter& emitter) {
        emitter.addComponent<ComponentModel>(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), Material::Checkers);
        auto& modelComponent = *emitter.getComponent<ComponentModel>();
        modelComponent.getModel().setScale(0.01f, 0.01f, 0.1f);
        modelComponent.getModel().translate(0.0f, 0.0f, 0.1f);
    }
}


ParticleEmitter::ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) 
    : Entity{ scene }
{
    addComponent<ComponentTransform>();

    //internal_add_debugging_visual(*this);

    init(properties, scene, lifetime, parent);
}
void ParticleEmitter::init(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    setProperties(properties);
    //removeAllChildren();
    getComponent<ComponentTransform>()->setPosition(std::numeric_limits<float>().max());
    if (parent) {
        parent.addChild(*this);
    }
    m_Lifetime      = lifetime;
    m_SpawningTimer = m_Properties->m_SpawnRate - 0.01f;

    //setLinearVelocity(0.0, 0.0, 0.0);
    getComponent<ComponentTransform>()->setScale(1.0, 1.0, 1.0);
}

ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept 
    : Entity         { std::move(other) }
    , m_UserData     { std::move(other.m_UserData) }
    , m_UpdateFunctor{ std::move(other.m_UpdateFunctor) }
    , m_Properties   { std::exchange(other.m_Properties, nullptr) }
    , m_SpawningTimer{ std::move(other.m_SpawningTimer) }
    , m_Timer        { std::move(other.m_Timer) }
    , m_Lifetime     { std::move(other.m_Lifetime) }
    , m_Parent       { std::exchange(other.m_Parent, Entity{}) }
    , m_Active       { std::exchange(other.m_Active, false) }
{}
ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& other) noexcept {  
    Entity::operator=(std::move(other));
    m_UserData      = std::move(other.m_UserData);
    m_UpdateFunctor = std::move(other.m_UpdateFunctor);
    m_Properties    = std::exchange(other.m_Properties, nullptr);
    m_SpawningTimer = std::move(other.m_SpawningTimer);
    m_Timer         = std::move(other.m_Timer);
    m_Lifetime      = std::move(other.m_Lifetime);
    m_Parent        = std::exchange(other.m_Parent, Entity{});
    m_Active        = std::exchange(other.m_Active, false);
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
        m_UpdateFunctor(this, dt, particleSystem.m_Mutex);
        if (m_Properties && m_SpawningTimer > m_Properties->m_SpawnRate) {
            //if (multi_threaded) {
                std::lock_guard lock{ particleSystem.m_Mutex };
                for (uint32_t i = 0; i < m_Properties->m_ParticlesPerSpawn; ++i) {
                    particleSystem.add_particle(*this);
                }
            //} else {
            //    for (uint32_t i = 0; i < m_Properties->m_ParticlesPerSpawn; ++i) {
            //        particleSystem.add_particle(*this);
            //    }
            //}
            m_SpawningTimer = 0.0f;
        }
        if (m_Lifetime > 0.0f && m_Timer >= m_Lifetime) {
            m_Active = false;
            m_Timer  = 0.0f;
            //if (multi_threaded) {
                std::lock_guard lock{ particleSystem.m_Mutex };
                particleSystem.m_ParticleEmitterFreelist.emplace_back(index);
            //} else {
            //    particleSystem.m_ParticleEmitterFreelist.emplace_back(index);
            //}
        }
    }
}

//void ParticleEmitter::applyLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    /*
    auto& transform = *getComponent<ComponentTransform>();
    const auto currVel = transform.getLinearVelocity();
    auto newVel = glm_vec3(x, y, z);
    if (local) {
        newVel = transform.getRotation() * newVel;
    }
    transform.setLinearVelocity(currVel + newVel, false);
    */
//}
//void ParticleEmitter::applyLinearVelocity(glm_vec3& velocity, const bool local) {
    /*
    auto& transform = *getComponent<ComponentTransform>();
    const auto currVel = transform.getLinearVelocity();
    if (local) {
        velocity = transform.getRotation() * velocity;
    }
    transform.setLinearVelocity(currVel + velocity, false);
    */
//}

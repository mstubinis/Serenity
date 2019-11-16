#include "Sparks.h"
#include <core/engine/utils/Utils.h>
#include <core/engine/materials/Material.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/math/Engine_Math.h>
#include "../ResourceManifest.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/compatibility.hpp>

using namespace std;
using namespace Engine;

Handle Sparks::SparksMaterial1;

ParticleEmissionProperties* Sparks::Spray = nullptr;
ParticleEmissionProperties* Sparks::Burst = nullptr;

struct SprayColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    //auto& maxLife = properties.getLifetime();
    //const auto alpha = glm::min(static_cast<float>(maxLife - particle_lifetime), 1.0f);
    return glm::vec4(0.92f, 0.865f, 0.477f, 1.0f);
}};
struct SprayAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    auto& scene = emitter->entity().scene();
    const auto particlePos = particle.position() + particle.m_Data.m_Velocity;
    const auto emitterPos = emitter->position();
    auto& camera = *scene.getActiveCamera();

    const auto particle_screen_pos = Math::getScreenCoordinates(particlePos, camera);
    const auto emitter_screen_pos = Math::getScreenCoordinates(emitterPos, camera);
    particle.m_Data.m_AngularVelocity = 0.0f;
    particle.m_Data.m_Angle = 0.0f;
    auto angle = std::atan2f(particle_screen_pos.y - emitter_screen_pos.y, particle_screen_pos.x - emitter_screen_pos.x) + 1.5708f;
    return angle;
}};
struct SprayVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec3(0.0f);
}};
struct SprayScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec2(0.0f);
}};
struct SprayDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return static_cast<float>(particle_lifetime * 0.001);
}};


struct SprayInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_y = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_z = (static_cast<float>((rand() % 10) + 40) / 100.0f);

    const auto final_x = random_amount_x * 0.12f;
    const auto final_y = random_amount_y * 0.12f;
    const auto final_z = random_amount_z * 0.72f;
    return glm::vec3(final_x, final_y, final_z);
}};
struct SprayInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.036f);
}};
struct SprayInitialAngularFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    auto& scene = emitter.entity().scene();
    const auto particlePos = particle.position() + particle.m_Data.m_Velocity;
    const auto emitterPos = emitter.position();
    auto& camera = *scene.getActiveCamera();

    const auto particle_screen_pos = Math::getScreenCoordinates(particlePos, camera, true);
    const auto emitter_screen_pos = Math::getScreenCoordinates(emitterPos, camera, true);
    particle.m_Data.m_AngularVelocity = 0.0f;

    auto atan2res = std::atan2f(particle_screen_pos.y - emitter_screen_pos.y, particle_screen_pos.x - emitter_screen_pos.x);
    auto angle = atan2res + 1.5708f;
    particle.m_Data.m_Angle = angle;
    return angle;
}};


void Sparks::init() {
    SparksMaterial1 = Resources::loadMaterialAsync("Sparks1", "../data/Textures/Effects/sparks_1.dds");
    ((Material*)SparksMaterial1.get())->setShadeless(true);
    ((Material*)SparksMaterial1.get())->setGlow(1.0f);


    Spray = new ParticleEmissionProperties(Sparks::SparksMaterial1, 0.4, 0.005, 1, 1.0f);


    Spray->setColorFunctor(SprayColorFunctor());
    Spray->setDepthFunctor(SprayDepthFunctor());
    Spray->setChangeInScaleFunctor(SprayScaleFunctor());
    Spray->setChangeInAngularVelocityFunctor(SprayAngularVelocityFunctor());
    Spray->setChangeInVelocityFunctor(SprayVelocityFunctor());

    Spray->setInitialVelocityFunctor(SprayInitialVelocityFunctor());
    Spray->setInitialScaleFunctor(SprayInitialScaleFunctor());
    Spray->setInitialAngularVelocityFunctor(SprayInitialAngularFunctor());


    Burst = new ParticleEmissionProperties(Sparks::SparksMaterial1, 0.4, 1.005, 15, 0.01f);


    Burst->setColorFunctor(SprayColorFunctor());
    Burst->setDepthFunctor(SprayDepthFunctor());
    Burst->setChangeInScaleFunctor(SprayScaleFunctor());
    Burst->setChangeInAngularVelocityFunctor(SprayAngularVelocityFunctor());
    Burst->setChangeInVelocityFunctor(SprayVelocityFunctor());

    Burst->setInitialVelocityFunctor(SprayInitialVelocityFunctor());
    Burst->setInitialScaleFunctor(SprayInitialScaleFunctor());
    Burst->setInitialAngularVelocityFunctor(SprayInitialAngularFunctor());
}
void Sparks::destruct() {
    SAFE_DELETE(Spray);
    SAFE_DELETE(Burst);
}
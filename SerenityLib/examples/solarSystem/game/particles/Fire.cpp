#include "Fire.h"
#include <core/engine/utils/Utils.h>
#include <core/engine/materials/Material.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include "../ResourceManifest.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

using namespace std;
using namespace Engine;

Handle Fire::SmokeMaterial1;
Handle Fire::SmokeMaterial2;
Handle Fire::SmokeMaterial3;

ParticleEmissionProperties* Fire::Regular = nullptr;
ParticleEmissionProperties* Fire::ShortLived = nullptr;

#define FIRE_COLOR_CUTOFF 0.45

struct FireColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    auto& maxLife = properties.getLifetime();

    float red, green;
    const auto alpha = glm::min(static_cast<float>(maxLife - particle_lifetime), 1.0f);
    //yellow to red very quickly then to black very quickly

    auto factor = particle_lifetime / FIRE_COLOR_CUTOFF; //0 to 1
    //yellow to red
    red = glm::min(1.0f, static_cast<float>(2.0 - factor));
    green = static_cast<float>(1.0 - factor);
    green = glm::clamp(green, 0.05f, 1.0f);
    if (particle_lifetime > FIRE_COLOR_CUTOFF) {
        //blackish now
        auto factor1 = (particle_lifetime - FIRE_COLOR_CUTOFF);
        red -= static_cast<float>(factor1);
        red = glm::clamp(red, 0.05f, 1.0f);
    }
    return glm::vec4(red, green, 0.05f, alpha);
}};
struct FireAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct FireVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec3(0.0f);
}};
struct FireScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec2(0.0f);
}};
struct FireDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return static_cast<float>(particle_lifetime * 0.001);
}};


struct FireInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_y = (static_cast<float>((rand() % 10)  + 40) / 100.0f); 
    const auto random_amount_z = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    return glm::vec3(random_amount_x * 0.03f, random_amount_y * 0.22f, random_amount_z * 0.03f);
}};
struct FireInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.076f);
}};
struct FireInitialAngularFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto first = (rand() % 101) - 50; //-50 to 50
    const float random_amount = glm::clamp(static_cast<float>(first) / 50.0f, -1.0f, 1.0f); //-1 to 1
    return 0.01f * random_amount;
}};


void Fire::init() {
    SmokeMaterial1 = Resources::loadMaterialAsync("Smoke1", "../data/Textures/Effects/smoke_1.dds");
    SmokeMaterial2 = Resources::loadMaterialAsync("Smoke2", "../data/Textures/Effects/smoke_2.dds");
    SmokeMaterial3 = Resources::loadMaterialAsync("Smoke3", "../data/Textures/Effects/smoke_3.dds");
    ((Material*)SmokeMaterial1.get())->setShadeless(true);
    ((Material*)SmokeMaterial2.get())->setShadeless(true);
    ((Material*)SmokeMaterial3.get())->setShadeless(true);
    ((Material*)SmokeMaterial1.get())->setGlow(1.0f);
    ((Material*)SmokeMaterial2.get())->setGlow(1.0f);
    ((Material*)SmokeMaterial3.get())->setGlow(1.0f);


    Regular = new ParticleEmissionProperties(Fire::SmokeMaterial1, 5.0, 0.02, 1, 1.0f);
    Regular->addMaterial(Fire::SmokeMaterial2);
    Regular->addMaterial(Fire::SmokeMaterial3);

    Regular->setColorFunctor(FireColorFunctor());
    Regular->setDepthFunctor(FireDepthFunctor());
    Regular->setChangeInScaleFunctor(FireScaleFunctor());
    Regular->setChangeInAngularVelocityFunctor(FireAngularVelocityFunctor());
    Regular->setChangeInVelocityFunctor(FireVelocityFunctor());

    Regular->setInitialVelocityFunctor(FireInitialVelocityFunctor());
    Regular->setInitialScaleFunctor(FireInitialScaleFunctor());
    Regular->setInitialAngularVelocityFunctor(FireInitialAngularFunctor());


    ShortLived = new ParticleEmissionProperties(Fire::SmokeMaterial1, 3.2, 0.023, 1, 1.0f);
    ShortLived->addMaterial(Fire::SmokeMaterial2);
    ShortLived->addMaterial(Fire::SmokeMaterial3);

    ShortLived->setColorFunctor(FireColorFunctor());
    ShortLived->setDepthFunctor(FireDepthFunctor());
    ShortLived->setChangeInScaleFunctor(FireScaleFunctor());
    ShortLived->setChangeInAngularVelocityFunctor(FireAngularVelocityFunctor());
    ShortLived->setChangeInVelocityFunctor(FireVelocityFunctor());

    ShortLived->setInitialVelocityFunctor(FireInitialVelocityFunctor());
    ShortLived->setInitialScaleFunctor(FireInitialScaleFunctor());
    ShortLived->setInitialAngularVelocityFunctor(FireInitialAngularFunctor());
}
void Fire::destruct() {
    SAFE_DELETE(Regular);
    SAFE_DELETE(ShortLived);
}
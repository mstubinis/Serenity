#include "Fire.h"
#include <core/engine/utils/Utils.h>
#include <core/engine/materials/Material.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include "../ResourceManifest.h"
#include "../Helper.h"

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
ParticleEmissionProperties* Fire::OutwardFireballDebrisFire = nullptr;
ParticleEmissionProperties* Fire::OutwardFireball = nullptr;

#define FIRE_COLOR_CUTOFF 0.4

#pragma region Fire

struct FireColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    auto& maxLife = properties.getLifetime();

    float red, green;
    auto lower_clamp = 0.05f * particle.m_Data.m_UserData.x;
    const auto alpha = glm::min(static_cast<float>(maxLife - particle_lifetime), 1.0f);
    //yellow to red very quickly then to black very quickly

    auto factor = particle_lifetime / FIRE_COLOR_CUTOFF; //0 to 1
    //yellow to red
    red = glm::min(1.0f, static_cast<float>(2.0 - factor));
    green = static_cast<float>(1.0 - factor);
    green = glm::clamp(green, lower_clamp, 1.0f);
    if (particle_lifetime > FIRE_COLOR_CUTOFF) {
        //blackish now
        auto factor1 = (particle_lifetime - FIRE_COLOR_CUTOFF);
        red -= static_cast<float>(factor1);
        red = glm::clamp(red, lower_clamp, 1.0f);
    }
    return glm::vec4(red, green, lower_clamp, alpha);
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
    return 0.0f;
}};


struct FireInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_y = (static_cast<float>((rand() % 101) - 50) / 50.0f); 
    const auto random_amount_z = (static_cast<float>((rand() % 10) + 40) / 100.0f);

    const auto final_x = random_amount_x * 0.03f;
    const auto final_y = random_amount_y * 0.03f;
    const auto final_z = random_amount_z * 0.22f;
    data.m_UserData.x = Helper::GetRandomFloatFromTo(0.8f, 1.2f);
    return glm::vec3(final_x, final_y, final_z);
}};
struct FireInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.076f);
}};
struct FireInitialAngularFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto first = (rand() % 101) - 50; //-50 to 50
    const float random_amount = glm::clamp(static_cast<float>(first) / 50.0f, -1.0f, 1.0f); //-1 to 1
    return 0.01f * random_amount;
}};
#pragma endregion

#pragma region OutwardFireballDebris

struct OutwardDebrisFireColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    auto& maxLife = properties.getLifetime();

    float red, green, blue;
    auto lower_clamp = 0.05f * particle.m_Data.m_UserData.x;
    const auto alpha = glm::min(static_cast<float>(maxLife - particle_lifetime), 1.0f);

    red = 1.0f;
    green = 0.62f;
    blue = 0.05f;
    auto factor = particle_lifetime * 2.0;
    red -= static_cast<float>(factor);
    red = glm::clamp(red, lower_clamp, 1.0f);
    green -= static_cast<float>(factor) * 1.1;
    green = glm::clamp(green, lower_clamp, 1.0f);
    blue -= static_cast<float>(factor);
    blue = glm::clamp(blue, lower_clamp, 1.0f) * 1.2;
    return glm::vec4(red, green, blue, alpha);
}};
struct OutwardDebrisAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct OutwardDebrisVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec3(0.0f);
}};
struct OutwardDebrisScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec2(0.0f);
}};
struct OutwardDebrisDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct OutwardDebrisInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_y = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_z = (static_cast<float>((rand() % 10) + 40) / 100.0f);

    const auto final_x = random_amount_x * 0.005f;
    const auto final_y = random_amount_y * 0.005f;
    const auto final_z = random_amount_z * 0.012f;
    data.m_UserData.x = Helper::GetRandomFloatFromTo(0.8f, 1.2f);
    return glm::vec3(final_x, final_y, final_z);
}};

struct OutwardDebrisInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.07f);
}};
struct OutwardDebrisSmokeInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.08f);
}};
struct OutwardDebrisInitialAngularFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto first = (rand() % 101) - 50; //-50 to 50
    const float random_amount = glm::clamp(static_cast<float>(first) / 50.0f, -1.0f, 1.0f); //-1 to 1
    return 0.015f * random_amount;
}};
#pragma endregion

#pragma region OutwardFireball
struct OutwardFireballFireColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    auto& maxLife = properties.getLifetime();

    float red, green, blue;
    auto lower_clamp = 0.05f * particle.m_Data.m_UserData.x;
    const auto alpha = glm::min(static_cast<float>(maxLife - particle_lifetime), 1.0f);

    red = 1.0f;
    green = 0.62f;
    blue = 0.05f;
    auto factor = particle_lifetime * 2.0;
    red -= static_cast<float>(factor);
    red = glm::clamp(red, lower_clamp, 1.0f);
    green -= static_cast<float>(factor) * 1.1;
    green = glm::clamp(green, lower_clamp, 1.0f);
    blue -= static_cast<float>(factor);
    blue = glm::clamp(blue, lower_clamp, 1.0f) * 1.2;
    return glm::vec4(red, green, blue, alpha);
}};
struct OutwardFireballAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct OutwardFireballVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec3(0.0f);
}};
struct OutwardFireballScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return glm::vec2(0.0f);
}};
struct OutwardFireballDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt, ParticleEmitter* emitter, Particle& particle) const {
    return 0.0f;
}};
struct OutwardFireballInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_y = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_z = (static_cast<float>((rand() % 10) + 40) / 100.0f);

    const auto final_x = random_amount_x * 0.005f;
    const auto final_y = random_amount_y * 0.005f;
    const auto final_z = random_amount_z * 0.012f;
    data.m_UserData.x = Helper::GetRandomFloatFromTo(0.8f, 1.2f);
    return glm::vec3(final_x, final_y, final_z);
}};
struct OutwardFireballInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.07f);
}};
struct OutwardFireballSmokeInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    return glm::vec2(0.08f);
}};
struct OutwardFireballInitialAngularFunctor final { float operator()(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle, ParticleData& data) const {
    const auto first = (rand() % 101) - 50; //-50 to 50
    const float random_amount = glm::clamp(static_cast<float>(first) / 50.0f, -1.0f, 1.0f); //-1 to 1
    return 0.015f * random_amount;
}};
#pragma endregion

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

    {
        Regular = new ParticleEmissionProperties(Fire::SmokeMaterial1, 5.0, 0.02, 1, 1.0f);
        //Regular->addMaterial(Fire::SmokeMaterial2);
        //Regular->addMaterial(Fire::SmokeMaterial3);
        Regular->setColorFunctor(FireColorFunctor());
        Regular->setDepthFunctor(FireDepthFunctor());
        Regular->setChangeInScaleFunctor(FireScaleFunctor());
        Regular->setChangeInAngularVelocityFunctor(FireAngularVelocityFunctor());
        Regular->setChangeInVelocityFunctor(FireVelocityFunctor());
        Regular->setInitialVelocityFunctor(FireInitialVelocityFunctor());
        Regular->setInitialScaleFunctor(FireInitialScaleFunctor());
        Regular->setInitialAngularVelocityFunctor(FireInitialAngularFunctor());
    }
    {
        ShortLived = new ParticleEmissionProperties(Fire::SmokeMaterial1, 3.2, 0.023, 1, 1.0f);
        //ShortLived->addMaterial(Fire::SmokeMaterial2);
        //ShortLived->addMaterial(Fire::SmokeMaterial3);
        ShortLived->setColorFunctor(FireColorFunctor());
        ShortLived->setDepthFunctor(FireDepthFunctor());
        ShortLived->setChangeInScaleFunctor(FireScaleFunctor());
        ShortLived->setChangeInAngularVelocityFunctor(FireAngularVelocityFunctor());
        ShortLived->setChangeInVelocityFunctor(FireVelocityFunctor());
        ShortLived->setInitialVelocityFunctor(FireInitialVelocityFunctor());
        ShortLived->setInitialScaleFunctor(FireInitialScaleFunctor());
        ShortLived->setInitialAngularVelocityFunctor(FireInitialAngularFunctor());
    }

    {
        OutwardFireballDebrisFire = new ParticleEmissionProperties(Fire::SmokeMaterial1, 4.0f, 0.04, 1, 1.0f);
        //OutwardFireballDebrisFire->addMaterial(Fire::SmokeMaterial2);
        //OutwardFireballDebrisFire->addMaterial(Fire::SmokeMaterial3);
        OutwardFireballDebrisFire->setColorFunctor(OutwardDebrisFireColorFunctor());
        OutwardFireballDebrisFire->setDepthFunctor(OutwardDebrisDepthFunctor());
        OutwardFireballDebrisFire->setChangeInScaleFunctor(OutwardDebrisScaleFunctor());
        OutwardFireballDebrisFire->setChangeInAngularVelocityFunctor(OutwardDebrisAngularVelocityFunctor());
        OutwardFireballDebrisFire->setChangeInVelocityFunctor(OutwardDebrisVelocityFunctor());
        OutwardFireballDebrisFire->setInitialVelocityFunctor(OutwardDebrisInitialVelocityFunctor());
        OutwardFireballDebrisFire->setInitialScaleFunctor(OutwardDebrisInitialScaleFunctor());
        OutwardFireballDebrisFire->setInitialAngularVelocityFunctor(OutwardDebrisInitialAngularFunctor());
    }

    {
        OutwardFireball = new ParticleEmissionProperties(Fire::SmokeMaterial1, 0.9f, 0.04, 1, 1.0f);
        //OutwardFireball->addMaterial(Fire::SmokeMaterial2);
        //OutwardFireball->addMaterial(Fire::SmokeMaterial3);
        OutwardFireball->setColorFunctor(OutwardFireballFireColorFunctor());
        OutwardFireball->setDepthFunctor(OutwardFireballDepthFunctor());
        OutwardFireball->setChangeInScaleFunctor(OutwardFireballScaleFunctor());
        OutwardFireball->setChangeInAngularVelocityFunctor(OutwardFireballAngularVelocityFunctor());
        OutwardFireball->setChangeInVelocityFunctor(OutwardFireballVelocityFunctor());
        OutwardFireball->setInitialVelocityFunctor(OutwardFireballInitialVelocityFunctor());
        OutwardFireball->setInitialScaleFunctor(OutwardFireballInitialScaleFunctor());
        OutwardFireball->setInitialAngularVelocityFunctor(OutwardFireballInitialAngularFunctor());
    }
}
void Fire::destruct() {
    SAFE_DELETE(Regular);
    SAFE_DELETE(ShortLived);
    SAFE_DELETE(OutwardFireballDebrisFire);
    SAFE_DELETE(OutwardFireball);
}
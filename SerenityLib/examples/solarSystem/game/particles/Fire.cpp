#include "Fire.h"
#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/ParticleEmissionProperties.h>
#include "../ResourceManifest.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

using namespace std;

ParticleEmissionProperties* Fire::m_Properties = nullptr;

#define FIRE_COLOR_CUTOFF 0.45

struct FireColorFunctor final { glm::vec4 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt) const {
    auto& maxLife = properties.getLifetime();

    float red, green;
    const auto alpha = glm::min(static_cast<float>(maxLife - particle_lifetime), 1.0f);
    //yellow to red very quickly then to black very quickly

    auto factor = particle_lifetime / FIRE_COLOR_CUTOFF; //0 to 1
    //yellow to red
    red = glm::min(1.0f, static_cast<float>(2.0 - factor));
    green = 1.0 - factor;
    green = glm::clamp(green, 0.05f, 1.0f);
    if (particle_lifetime > FIRE_COLOR_CUTOFF) {
        //blackish now
        auto factor1 = (particle_lifetime - FIRE_COLOR_CUTOFF);
        red -= factor1;
        red = glm::clamp(red, 0.05f, 1.0f);
    }
    return glm::vec4(red, green, 0.05f, alpha);
}};
struct FireAngularVelocityFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt) const {
    return 0.0f;
}};
struct FireVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt) const {
    return glm::vec3(0.0f);
}};
struct FireScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt) const {
    return glm::vec2(0.0f);
}};
struct FireDepthFunctor final { float operator()(ParticleEmissionProperties& properties, const double& particle_lifetime, const double& dt) const {
    return static_cast<float>(particle_lifetime * 0.001);
}};


struct FireInitialVelocityFunctor final { glm::vec3 operator()(ParticleEmissionProperties& properties) const {
    const auto random_amount_x = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    const auto random_amount_y = (static_cast<float>((rand() % 10)  + 40) / 100.0f); 
    const auto random_amount_z = (static_cast<float>((rand() % 101) - 50) / 50.0f);
    return glm::vec3(random_amount_x * 0.03f, random_amount_y * 0.22f, random_amount_z * 0.03f);
}};
struct FireInitialScaleFunctor final { glm::vec2 operator()(ParticleEmissionProperties& properties) const {
    return glm::vec2(0.22f);
}};
struct FireInitialAngularFunctor final { float operator()(ParticleEmissionProperties& properties) const {
    const auto first = (rand() % 101) - 50; //-50 to 50
    const float random_amount = glm::clamp(static_cast<float>(first) / 50.0f, -1.0f, 1.0f); //-1 to 1
    return 0.01f * random_amount;
}};


void Fire::init() {
    m_Properties = new ParticleEmissionProperties(ResourceManifest::SmokeMaterial1, 5.0, 0.02);
    m_Properties->addMaterial(ResourceManifest::SmokeMaterial2);
    m_Properties->addMaterial(ResourceManifest::SmokeMaterial3);

    m_Properties->setColorFunctor(FireColorFunctor());
    m_Properties->setDepthFunctor(FireDepthFunctor());
    m_Properties->setChangeInScaleFunctor(FireScaleFunctor());
    m_Properties->setChangeInAngularVelocityFunctor(FireAngularVelocityFunctor());
    m_Properties->setChangeInVelocityFunctor(FireVelocityFunctor());

    m_Properties->setInitialVelocityFunctor(FireInitialVelocityFunctor());
    m_Properties->setInitialScaleFunctor(FireInitialScaleFunctor());
    m_Properties->setInitialAngularVelocityFunctor(FireInitialAngularFunctor());
}
void Fire::destruct() {
    SAFE_DELETE(m_Properties);
}
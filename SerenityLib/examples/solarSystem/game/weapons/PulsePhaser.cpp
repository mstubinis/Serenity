#include "PulsePhaser.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"

#include <ecs/Components.h>
#include <core/engine/math/Engine_Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/mesh/Mesh.h>

PulsePhaserProjectile::PulsePhaserProjectile(PulsePhaser& source, Map& map, const glm::vec3& position, const glm::vec3& forward) {
    entity = map.createEntity();
    currentTime = 0.0f;
    maxTime = 5.0f;
    active = true;

    auto& model = *entity.addComponent<ComponentModel>(
        ResourceManifest::CannonEffectMesh,
        ResourceManifest::PulsePhaserMaterial,
        ShaderProgram::Forward,
        RenderStage::ForwardParticles
    );
    auto& body = *entity.addComponent<ComponentBody>(CollisionType::Box);

    Ship& s = source.ship;
    auto& shipBody = *s.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    auto shipRotation = shipBody.rotation();
    shipMatrix = glm::translate(shipMatrix, position + glm::vec3(0,0,-model.getModel().mesh()->getRadiusBox().z * 1.6f));
    glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    body.setPosition(finalPosition);
    body.setRotation(shipRotation);
    body.addCollisionFlag(CollisionFlag::NoContactResponse);
    body.setLinearVelocity(0.0f, 0.0f, -50.5f, true);
}
PulsePhaserProjectile::~PulsePhaserProjectile() {

}
void PulsePhaserProjectile::update(const double& dt) {
    if (active) {
        currentTime += dt;
        if (currentTime >= maxTime) {
            active = false;
            entity.destroy();
        }
    }
}

PulsePhaser::PulsePhaser(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc):PrimaryWeaponCannon(ship,position,forward,arc), m_Map(map){

}
PulsePhaser::~PulsePhaser() {

}

void PulsePhaser::update(const double& dt) {
    for (auto& projectile : m_ActiveProjectiles) {
        if (!projectile->active) {
            removeFromVector(m_ActiveProjectiles, projectile);
        }
    }
    for (auto& projectile : m_ActiveProjectiles) {
        projectile->update(dt);
    }
}
bool PulsePhaser::fire() {
    auto* projectile = new PulsePhaserProjectile(*this, m_Map, position, forward); 
    m_ActiveProjectiles.push_back(projectile);
    auto sound = Engine::Sound::playEffect(ResourceManifest::SoundPulsePhaser);


    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    if (sound) {
        sound->setPosition(finalPosition);
        sound->setAttenuation(0.15f);
    }
    return true;
}
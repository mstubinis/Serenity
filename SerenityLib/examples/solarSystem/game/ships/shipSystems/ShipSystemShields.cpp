#include "ShipSystemShields.h"
#include "../../Ship.h"
#include "../../map/Map.h"
#include "../../ResourceManifest.h"
#include "../../Helper.h"

#include <core/ModelInstance.h>
#include <ecs/Components.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/scene/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

struct ShipSystemShieldsFunctor final { void operator()(ComponentLogic& _component, const double& dt) const {

}};

struct ShieldInstanceBindFunctor {void operator()(EngineResource* r) const {
    auto& i = *static_cast<ModelInstance*>(r);
    auto& shields = *static_cast<ShipSystemShields*>(i.getUserPointer());
    const auto& stage = i.stage();
    auto& scene = *Resources::getCurrentScene();
    Camera& cam = *scene.getActiveCamera();
    glm::vec3 camPos = cam.getPosition();
    Entity& parent = i.parent();
    auto& body = *(parent.getComponent<ComponentBody>());
    glm::mat4 parentModel = body.modelMatrix();

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());

    Renderer::sendUniform1Safe("AnimationPlaying", 0);

    glm::mat4 modelMatrix = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
    int count = 0;
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        auto& impact = shields.m_ImpactPoints[i];
        if (impact.active) {
            glm::mat4 positionMatrix = modelMatrix;
            positionMatrix = glm::translate(positionMatrix, impact.impactLocation);
            glm::vec3 newPos = glm::vec3(positionMatrix[3][0], positionMatrix[3][1], positionMatrix[3][2]);
            Renderer::sendUniform3Safe(("impacts[" + to_string(count) + "].Position").c_str(), newPos);
            Renderer::sendUniform3Safe(("impacts[" + to_string(count) + "].Data").c_str(), impact.impactRadius, impact.currentTime, impact.maxTime);
            ++count;
        }
    }
    Renderer::sendUniform1Safe("numImpacts", count);
}};

ShipSystemShields::ShipSystemShields(Ship& _ship, Map* map) :ShipSystem(ShipSystemType::Shields, _ship), m_ShieldEntity(*map) {
    auto& model = *(m_ShieldEntity.addComponent<ComponentModel>(ResourceManifest::ShieldMesh, ResourceManifest::ShieldMaterial, ResourceManifest::shieldsShaderProgram, RenderStage::ForwardParticles));
    auto& logic = *(m_ShieldEntity.addComponent<ComponentLogic>(ShipSystemShieldsFunctor()));
    auto& body = *(m_ShieldEntity.addComponent<ComponentBody>(CollisionType::Sphere));
    logic.setUserPointer(&m_Ship);
    //logic.setUserPointer1(&model);
    //logic.setUserPointer2(&instance);


    auto& boundBox = m_Ship.getComponent<ComponentModel>()->getModel(0).mesh()->getRadiusBox();
    //note: add collision flags -- shields should not collide with other shields or ships. but SHOULD collide with projectiles
    auto& btBody = const_cast<btRigidBody&>(body.getBody());
    body.addCollisionFlag(CollisionFlag::NoContactResponse);

    btBody.setUserPointer(&m_Ship);
    body.setScale((boundBox.x * 1.37f), (boundBox.y * 1.37f), (boundBox.z * 1.37f)); //todo: fix this for when other ships are created


    auto& handles = ResourceManifest::Ships.at(m_Ship.getClass());
    auto& r = handles.get<3>().r;
    auto& g = handles.get<3>().g;
    auto& b = handles.get<3>().b;
    auto& instance = model.getModel(0);
    instance.setColor(r, g, b, 0.7f);
    instance.setCustomBindFunctor(ShieldInstanceBindFunctor());
    instance.setUserPointer(this);

    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        m_ImpactPoints[i].indexInArray = i;
    }
    m_ImpactPointsFreelist.reserve(MAX_IMPACT_POINTS);
    for (uint i = 0; i < MAX_IMPACT_POINTS; ++i) {
        m_ImpactPointsFreelist.push_back(i);
    }
}
ShipSystemShields::~ShipSystemShields() {

}
void ShipSystemShields::update(const double& dt) {
    auto& shieldBody = *m_ShieldEntity.getComponent<ComponentBody>();
    auto& shipBody = *m_Ship.getComponent<ComponentBody>();
    shieldBody.setPosition(shipBody.position());
    shieldBody.setRotation(shipBody.rotation());
    
    if (Engine::isKeyDownOnce(KeyboardKey::M)) {
        auto& boundBox = m_Ship.getComponent<ComponentModel>()->getModel(0).mesh()->getRadiusBox();

        float randX = Helper::GetRandomFloatFromTo(-boundBox.x, boundBox.x);
        float randY = Helper::GetRandomFloatFromTo(-boundBox.y, boundBox.y);
        float randZ = Helper::GetRandomFloatFromTo(-boundBox.z, boundBox.z);
        receiveHit(glm::vec3(randX, randY, randZ), 0.8f, 2.4f, 100);
    }
    
    const float fdt = static_cast<float>(dt);
    for (auto& impact : m_ImpactPoints) {
        impact.update(fdt, m_ImpactPointsFreelist);
    }

    ShipSystem::update(dt);
}
void ShipSystemShields::receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const float& damage) {
    if (m_ImpactPointsFreelist.size() > 0) {
        auto nextIndex = m_ImpactPointsFreelist[0];
        auto& impactPointData = m_ImpactPoints[nextIndex];
        impactPointData.impact(impactLocation, impactRadius, maxTime, m_ImpactPointsFreelist);
    }
}
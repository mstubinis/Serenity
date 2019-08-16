#include "ShipSystemSensors.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../map/Map.h"

#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>

using namespace Engine;
using namespace std;

ShipSystemSensors::ShipSystemSensors(Ship& _ship, Map& map) :ShipSystem(ShipSystemType::Sensors, _ship),m_Map(map) {

}
ShipSystemSensors::~ShipSystemSensors() {

}
void ShipSystemSensors::update(const double& dt) {
    if (m_Ship.getTarget()) {
        Ship* target = dynamic_cast<Ship*>(m_Ship.getTarget());
        if (target) {
            if (target->isFullyCloaked()) {
                m_Ship.setTarget(nullptr, true);
            }
        }
    }
    ShipSystem::update(dt);
}
void ShipSystemSensors::render() {
    const auto& winSize = Resources::getWindowSize();

    auto& radarMat = *(Material*)ResourceManifest::RaderMaterial.get();
    auto& radarEdgeMat = *(Material*)ResourceManifest::RadarEdgeMaterial.get();
    auto& radarTokenMat = *(Material*)ResourceManifest::RadarTokenMaterial.get();


    const auto& radarTexture = *radarMat.getComponent(0).texture();
    const auto& radarEdgeTexture = *radarEdgeMat.getComponent(0).texture();
    const auto& radarTokenTexture = *radarTokenMat.getComponent(0).texture();
    const auto& radarTextureWidth = radarTexture.width();
    const auto& radarTextureHalfWidth = static_cast<float>(radarTexture.height()) / 2.0f;
    auto radarPos = glm::vec2(winSize.x / 2, 0);
    auto radarPosAbs = glm::vec2(radarPos.x, radarPos.y + radarTextureHalfWidth);
    //render radar itself
    Renderer::renderTexture(radarTexture, radarPos,glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.0f), 0.2f, Alignment::BottomCenter);
    //render radar edge
    Renderer::renderTexture(radarEdgeTexture, glm::vec2(winSize.x / 2, 0), glm::vec4(1, 1, 0, 1), 0, glm::vec2(1.0f), 0.17f, Alignment::BottomCenter);

    const auto& myBody = m_Ship.getComponent<ComponentBody>();
    const auto& myPos = myBody->position();
    auto& camera = *myBody->getOwner().scene().getActiveCamera();
    const auto view = glm::lookAt(myPos, myPos - myBody->forward(), myBody->up());
    auto* myTarget = m_Ship.getTarget();
    for (auto& ship : m_Map.getShips()) {
        if (ship.first != m_Ship.getName() && !ship.second->isFullyCloaked()) {
            const auto& otherPos = ship.second->getPosition();
            const auto viewport = glm::vec4( (winSize.x / 2) - radarTextureHalfWidth, 0, radarTextureWidth, radarTexture.height());
            auto pos = Math::getScreenCoordinates(otherPos, camera, view, camera.getProjection(), viewport, false);
            glm::vec2 pos2D = glm::vec2(pos.x, pos.y);
            //now restrict pos2D to the circle
            auto offset = pos2D - radarPosAbs;
            if (glm::length(offset) > radarTextureHalfWidth) {
                pos2D = glm::normalize(offset) * radarTextureHalfWidth;
                pos2D += radarPosAbs;
            }

            //render radar token
            if(pos.z > 0)
                Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 0, 0, 1), 0, glm::vec2(1.0f), 0.14f, Alignment::Center);
            else
                Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 0, 0, 0.5f), 0, glm::vec2(1.0f), 0.14f, Alignment::Center);

            if (myTarget && myTarget->getComponent<ComponentName>()->name() == ship.second->getName()) {
                Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.2f), 0.16f, Alignment::Center);
            }
        }
    }



}
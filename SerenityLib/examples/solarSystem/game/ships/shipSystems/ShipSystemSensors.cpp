#include "ShipSystemSensors.h"
#include "ShipSystemCloakingDevice.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../map/Map.h"
#include "../../Helper.h"

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
        if (ship.first != m_Ship.getName()) {
            const auto& otherPos = ship.second->getPosition();
            const auto viewport = glm::vec4( (winSize.x / 2) - radarTextureHalfWidth, 0, radarTextureWidth, radarTexture.height());
            auto pos = Math::getScreenCoordinates(otherPos, camera, view, camera.getProjection(), viewport, false);
            glm::vec2 pos2D = glm::vec2(pos.x, pos.y);

            glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); //red first (enemy)
            if (pos.z <= 0) {
                color.a = 0.5f;
            }
            auto* cloakingDevice = static_cast<ShipSystemCloakingDevice*>(ship.second->getShipSystem(ShipSystemType::CloakingDevice));
            if (cloakingDevice) {
                auto timer = cloakingDevice->getCloakTimer();
                //1.0 - see, 0.0 or below is invisible
                if (timer < 1.0f) {
                    timer = glm::max(0.0f, timer);
                    float invTimer = 1.0f - timer;
                    auto randX2 = Helper::GetRandomFloatFromTo(-2.0f, 2.0f);
                    auto randY2 = Helper::GetRandomFloatFromTo(-2.0f, 2.0f);
                    auto randX = Helper::GetRandomFloatFromTo((-12.0f + randX2) * invTimer, (12.0f + randX2) * invTimer);
                    auto randY = Helper::GetRandomFloatFromTo((-12.0f + randY2) * invTimer, (12.0f + randY2) * invTimer);
                    color.r = 1.0f; //cannot tell if cloaked vessel is enemy or not
                    color.g = 1.0f;
                    color.b = 0.0f;
                    color.a = glm::max(0.35f, 1.0f * timer);
                    pos2D += glm::vec2(randX, randY);
                }
            }


            //now restrict pos2D to the circle
            auto offset = pos2D - radarPosAbs;
            if (glm::length(offset) > radarTextureHalfWidth) {
                pos2D = glm::normalize(offset) * radarTextureHalfWidth;
                pos2D += radarPosAbs;
            }
            //render radar token
            Renderer::renderTexture(radarTokenTexture, pos2D, color, 0, glm::vec2(1.0f), 0.14f, Alignment::Center);

            if (myTarget && myTarget->getComponent<ComponentName>()->name() == ship.second->getName()) {
                Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.2f), 0.16f, Alignment::Center);
            }
        }
    }



}
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

ShipSystemSensors::ShipSystemSensors(Ship& _ship, Map& map, const float& range) :ShipSystem(ShipSystemType::Sensors, _ship),m_Map(map){
    auto& radarMat = *(Material*)ResourceManifest::RaderMaterial.get();
    const auto& radarTexture = *radarMat.getComponent(0).texture();
    const auto& winSize = Resources::getWindowSize();

    const auto halfWinWidth = winSize.x / 2.0f;
    const auto halfTextureWidth = radarTexture.width() / 2.0f;

    //m_Camera = new Camera(0.0f, static_cast<float>(winSize.x), 0.0f, static_cast<float>(winSize.y), 0.005f, 1000.0f, &m_Map);
    m_Camera = new Camera(60.0f, static_cast<float>(winSize.x) / static_cast<float>(winSize.y), 0.005f, 1000.0f, &m_Map);


    m_ViewportObject = &m_Map.addViewport(halfWinWidth - halfTextureWidth, 0, radarTexture.width(), radarTexture.height(), *m_Camera);
    m_ViewportObject->activate();
    m_ViewportObject->activate2DAPI(false);
    //m_ViewportObject->setSkyboxVisible(false);
    m_ViewportObject->activateDepthMask();
    m_ViewportObject->setDepthMaskValue(15);

    m_RadarRingEntity = map.createEntity();
    m_RadarRingEntity.addComponent<ComponentBody>();
    auto& radarModel = *m_RadarRingEntity.addComponent<ComponentModel>(ResourceManifest::RadarDiscMesh,Material::WhiteShadeless,ShaderProgram::Forward,RenderStage::ForwardTransparent);
    radarModel.getModel().setColor(1, 1, 0, 1);
    radarModel.getModel().setViewportFlag(ViewportFlag::_2);

    m_RadarRange = range;

    const auto& radarTextureWidth = radarTexture.width();
    const auto& radarTextureHalfWidth = static_cast<float>(radarTexture.height()) / 2.0f;
    m_Viewport = glm::vec4((winSize.x / 2) - radarTextureHalfWidth, 0, radarTextureWidth, radarTexture.height());
}
ShipSystemSensors::~ShipSystemSensors() {

}
const Entity& ShipSystemSensors::radarRingEntity() const {
    return m_RadarRingEntity;
}
const Entity& ShipSystemSensors::radarCameraEntity() const {
    return m_Camera->entity();
}
void ShipSystemSensors::onResize(const uint& width, const uint& height) {
    auto& radarMat = *(Material*)ResourceManifest::RaderMaterial.get();
    const auto& radarTexture = *radarMat.getComponent(0).texture();

    const auto halfWinWidth = width / 2.0f;
    const auto halfTextureWidth = radarTexture.width() / 2.0f;

    m_ViewportObject->setViewportDimensions(halfWinWidth - halfTextureWidth, 0, radarTexture.width(), radarTexture.height());
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

    auto& camBody = *m_Camera->getComponent<ComponentBody>();
    const auto shipForward = m_Ship.forward();
    const auto extendedForward = shipForward * 100000.0f;
    camBody.setPosition(m_Ship.getPosition() - extendedForward);
    camBody.setRotation(m_Ship.getRotation());
    const auto camPos = camBody.position();
    m_Camera->lookAt(camPos, camPos - extendedForward, m_Ship.up());

    auto& radarBody = *m_RadarRingEntity.getComponent<ComponentBody>();
    radarBody.setPosition(camPos - (shipForward * glm::vec3(2.05f)));

    ShipSystem::update(dt);
}
void ShipSystemSensors::render() {
    const auto& winSize = Resources::getWindowSize();

    auto& radarMat = *(Material*)ResourceManifest::RaderMaterial.get();
    auto& radarEdgeMat = *(Material*)ResourceManifest::RadarEdgeMaterial.get();
    auto& radarTokenMat = *(Material*)ResourceManifest::RadarTokenMaterial.get();

    const auto& radarTexture      = *radarMat.getComponent(0).texture();
    const auto& radarEdgeTexture  = *radarEdgeMat.getComponent(0).texture();
    const auto& radarTokenTexture = *radarTokenMat.getComponent(0).texture();
    auto radarPos = glm::vec2(winSize.x / 2, 0);
    //render radar 2d graphic
    Renderer::renderTexture(radarTexture, radarPos,glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.0f), 0.2f, Alignment::BottomCenter);
    //render radar edge
    Renderer::renderTexture(radarEdgeTexture, glm::vec2(winSize.x / 2, 0), glm::vec4(0.11f, 0.16f, 0.19f, 1), 0, glm::vec2(1.0f), 0.17f, Alignment::BottomCenter);

    const auto& radarBodyPosition = m_RadarRingEntity.getComponent<ComponentBody>()->position();
    const auto& myPos = m_Ship.getComponent<ComponentBody>()->position();
    auto* myTarget = m_Ship.getTarget();
    for (auto& ship : m_Map.getShips()) {
        if (ship.first != m_Ship.getName()) {
            glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); //red first (enemy)
            glm::vec3 otherVector = ship.second->getPosition() - myPos;
            auto* cloakingDevice = static_cast<ShipSystemCloakingDevice*>(ship.second->getShipSystem(ShipSystemType::CloakingDevice));
            bool modByCloak = false;
            if (cloakingDevice) {
                auto timer = cloakingDevice->getCloakTimer();
                //1.0 - see, 0.0 or below is invisible
                if (timer < 1.0f) {
                    timer = glm::max(0.0f, timer);
                    float invTimer = 1.0f - timer;
                    const auto _small = m_RadarRange * 0.03f;
                    const auto _large = m_RadarRange * 0.22f;
                    auto randX2 = Helper::GetRandomFloatFromTo(-_small, _small);
                    auto randY2 = Helper::GetRandomFloatFromTo(-_small, _small);
                    auto randZ2 = Helper::GetRandomFloatFromTo(-_small, _small);
                    auto randX = Helper::GetRandomFloatFromTo((-_large + randX2) * invTimer, (_large + randX2) * invTimer);
                    auto randY = Helper::GetRandomFloatFromTo((-_large + randY2) * invTimer, (_large + randY2) * invTimer);
                    auto randZ = Helper::GetRandomFloatFromTo((-_large + randZ2) * invTimer, (_large + randZ2) * invTimer);
                    color.r = 1.0f; //cannot tell if cloaked vessel is enemy or not
                    color.g = 1.0f;
                    color.b = 0.0f;
                    color.a = glm::max(0.35f, 1.0f * timer);
                    modByCloak = true;
                    otherVector += glm::vec3(randX, randY, randZ) * ((glm::length(otherVector) / m_RadarRange) + 0.01f);
                }
            }
            
            //scale otherPos down to the range
            const float otherLen = glm::length(otherVector);
            otherVector /= otherLen;
            otherVector *= glm::min((otherLen / m_RadarRange + 0.01f), 1.0f);
            otherVector = radarBodyPosition + otherVector;

            const auto pos = Math::getScreenCoordinates(otherVector, *m_Camera, m_Camera->getView(), m_Camera->getProjection(), m_Viewport, false);
            const glm::vec2 pos2D = glm::vec2(pos.x, pos.y);
            const float dotproduct = glm::dot(radarBodyPosition + m_Ship.forward(), otherVector - radarBodyPosition);
            if (dotproduct <= 0 /*&& !modByCloak*/) {
                color.a = 0.5f;
            }
            //render radar token
            Renderer::renderTexture(radarTokenTexture, pos2D, color, 0, glm::vec2(1.0f), 0.14f, Alignment::Center);

            if (myTarget && myTarget->getComponent<ComponentName>()->name() == ship.second->getName()) {
                Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.2f), 0.16f, Alignment::Center);
            }
        }
    }
}
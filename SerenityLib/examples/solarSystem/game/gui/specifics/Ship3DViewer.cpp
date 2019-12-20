#include "Ship3DViewer.h"
#include "../../GameCamera.h"
#include "../../ships/Ships.h"
#include "../../Core.h"
#include "../../map/Map.h"

#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>

using namespace std;
using namespace Engine;

struct GameCameraShipSelectorLogicFunctor final { void operator()(ComponentLogic2& _component, const double& dt) const {
    GameCamera& camera = *static_cast<GameCamera*>(_component.getUserPointer());
    Core& core = *static_cast<Core*>(_component.getUserPointer1());
    Ship3DViewer& window = *static_cast<Ship3DViewer*>(_component.getUserPointer2());
    if (core.gameState() == GameState::Host_Server_Lobby_And_Ship || core.gameState() == GameState::Join_Server_Server_Lobby) {
        auto& entity = camera.entity();
        EntityDataRequest dataRequest(entity);
        auto& thisCamera = *entity.getComponent<ComponentCamera>(dataRequest);
        auto& thisBody = *entity.getComponent<ComponentBody>(dataRequest);
        if ((window.m_IsCurrentlyOverShip3DWindow && dt >= 0.0) || window.m_IsCurrentlyDragging) {
            camera.m_OrbitRadius += static_cast<float>(Engine::getMouseWheelDelta() * dt * 0.92);
            camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 1.5f);
        }
        if (window.m_IsCurrentlyDragging || dt < 0.0) {
            const auto& diff = Engine::getMouseDifference();
            camera.m_CameraMouseFactor += glm::dvec2(-diff.y * (dt), diff.x * (dt));
            thisBody.rotate(camera.m_CameraMouseFactor.x, camera.m_CameraMouseFactor.y, 0.0);
            camera.m_CameraMouseFactor *= 0.0;
        }
        if (window.m_EntityWrapperShip) {
            auto& ship            = *window.m_EntityWrapperShip;
            EntityDataRequest dataRequest(ship.entity());

            auto& targetBody      = *ship.getComponent<ComponentBody>(dataRequest);
            auto& targetModel     = *ship.getComponent<ComponentModel>(dataRequest);
            auto& radius          = targetModel.radius();
            const glm::vec3 pos   = (glm::vec3(0.0f, 0.0f, 1.0f) * glm::length(radius) * 0.37f) + (glm::vec3(0.0f, 0.0f, 1.0f) * glm::length(radius * (1.0f + camera.m_OrbitRadius)));

            glm::mat4 cameraModel = glm::mat4(1.0f);
            cameraModel           = glm::translate(cameraModel, glm::vec3(targetBody.position()));
            cameraModel          *= glm::mat4_cast(glm::quat(thisBody.rotation()));
            cameraModel           = glm::translate(cameraModel, pos);

            const glm::vec3 eye = Math::getMatrixPosition(cameraModel);
            thisBody.setPosition(eye);
            thisCamera.lookAt(eye, targetBody.position(), thisBody.up());
        }
    }
}};

Ship3DViewer::Ship3DViewer(Core& core, Scene& menu_scene, Camera& game_camera, const float x, const float y, const float w, const float h) {
    m_Color                       = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    m_IsCurrentlyDragging         = false;
    m_IsCurrentlyOverShip3DWindow = false;
    m_ChosenShipClass             = "";
    GameCamera& menu_scene_camera = static_cast<GameCamera&>(game_camera);

    m_EntityWrapperShip = Map::createShipDull("Defiant", glm::vec3(0.0f), &menu_scene);
    menu_scene_camera.setTarget(m_EntityWrapperShip);
    auto& ship_body = *m_EntityWrapperShip->getComponent<ComponentBody>();
    ship_body.setPosition(0, 0, 8500);
    auto& ship_model = *m_EntityWrapperShip->getComponent<ComponentModel>();
    ship_model.setViewportFlag(ViewportFlag::All);
    ship_model.hide();

    auto& logic                   = *game_camera.getComponent<ComponentLogic2>();
    logic.setUserPointer1(&core);
    logic.setUserPointer2(this);
    logic.setFunctor(GameCameraShipSelectorLogicFunctor());

    m_ShipDisplayViewport         = &menu_scene.addViewport(x, y, w, h, game_camera);
    //m_ShipDisplayViewport->setDepthMaskValue(100.0f);
    //m_ShipDisplayViewport->activateDepthMask(true);
    m_ShipDisplayViewport->removeRenderFlag(ViewportRenderingFlag::API2D);
    m_ShipDisplayViewport->setAspectRatioSynced(true);

    auto& game_camera_body        = *game_camera.getComponent<ComponentBody>();
    game_camera_body.rotate(0.0f, glm::radians(190.0f), 0.0f, true);
    game_camera_body.rotate(glm::radians(10.0f), 0.0f, 0.0f, true);
}
Ship3DViewer::~Ship3DViewer() {
    SAFE_DELETE(m_EntityWrapperShip);
}
void Ship3DViewer::setPosition(const float x, const float y) {
    auto& dimensions = m_ShipDisplayViewport->getViewportDimensions();
    m_ShipDisplayViewport->setViewportDimensions(x, y, dimensions.z, dimensions.w);
}
void Ship3DViewer::setShipClass(const string& shipClass) {
    if (!m_EntityWrapperShip || m_ChosenShipClass == shipClass)
        return;
    assert(Ships::Database.count(shipClass));

    auto& scene = m_EntityWrapperShip->entity().scene();
    m_EntityWrapperShip->destroy();
    SAFE_DELETE(m_EntityWrapperShip);
    m_EntityWrapperShip = Map::createShipDull(shipClass, glm::vec3(0.0f), &scene);
    GameCamera& menu_scene_camera = static_cast<GameCamera&>(const_cast<Camera&>(m_ShipDisplayViewport->getCamera()));
    menu_scene_camera.setTarget(m_EntityWrapperShip);
    auto& ship_body = *m_EntityWrapperShip->getComponent<ComponentBody>();
    ship_body.setPosition(0, 0, 8500);
    auto& model = *m_EntityWrapperShip->getComponent<ComponentModel>();
    model.setViewportFlag(ViewportFlag::All);

    m_ChosenShipClass                 = shipClass;
    auto& shipData                    = Ships::Database.at(shipClass);
    model.show();
    auto* logic = menu_scene_camera.getComponent<ComponentLogic2>();
    if (logic)
        logic->call(-0.0001);
}
const string& Ship3DViewer::getShipClass() const {
    return m_ChosenShipClass;
}
void Ship3DViewer::setShipViewportActive(const bool& active) {
    m_ShipDisplayViewport->activate(active);
}
void Ship3DViewer::setUserPointer(void* UserPointer) {
    m_UserPointer = UserPointer;
}
void* Ship3DViewer::getUserPointer() {
    return m_UserPointer;
}
const Viewport& Ship3DViewer::getShipDisplay() const {
    return *m_ShipDisplayViewport;
}
void Ship3DViewer::update(const double& dt) {
    auto& dimensions              = m_ShipDisplayViewport->getViewportDimensions();
    auto& mouse                   = Engine::getMousePosition();
    m_IsCurrentlyOverShip3DWindow = true;
    if (mouse.x < dimensions.x || mouse.x > dimensions.x + dimensions.z || mouse.y < dimensions.y || mouse.y > dimensions.y + dimensions.w) {
        m_IsCurrentlyOverShip3DWindow = false;
    }
    if (!Engine::isMouseButtonDown(MouseButton::Left)) {
        m_IsCurrentlyDragging = false;
    }
    if (m_IsCurrentlyOverShip3DWindow && Engine::isMouseButtonDown(MouseButton::Left)) {
        m_IsCurrentlyDragging = true;
    }
}
void Ship3DViewer::render() {
    auto& dimensions = m_ShipDisplayViewport->getViewportDimensions();
    Renderer::renderBorder(1.0f, glm::vec2(dimensions.x + 1.0f, dimensions.y), m_Color, dimensions.z + 1.0f, dimensions.w, 0, 0.01f, Alignment::BottomLeft);
}
#include "ServerLobbyShipSelectorWindow.h"
#include <core/engine/Engine.h>
#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>

#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>

#include <ecs/Components.h>

#include <iostream>

#include "../../GameCamera.h"

#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../Core.h"

using namespace Engine;
using namespace std;

struct GameCameraShipSelectorLogicFunctor final { void operator()(ComponentLogic2& _component, const double& dt) const {
    GameCamera& camera = *static_cast<GameCamera*>(_component.getUserPointer());
    auto& core = *static_cast<Core*>(_component.getUserPointer1());
    ServerLobbyShipSelectorWindow& window = *static_cast<ServerLobbyShipSelectorWindow*>(_component.getUserPointer2());
    if (core.gameState() == GameState::Host_Server_Lobby_And_Ship || core.gameState() == GameState::Join_Server_Server_Lobby) {
        auto entity = camera.entity();
        EntityDataRequest dataRequest(entity);
        auto& thisCamera = *entity.getComponent<ComponentCamera>(dataRequest);
        auto& thisBody = *entity.getComponent<ComponentBody>(dataRequest);
        if ((window.m_IsCurrentlyOverShipWindow && dt >= 0.0) || window.m_IsCurrentlyDragging) {
            camera.m_OrbitRadius += static_cast<float>(Engine::getMouseWheelDelta() * dt * 0.92);
            camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 1.5f);
        }
        if (window.m_IsCurrentlyDragging || dt < 0.0) {
            const auto& diff = Engine::getMouseDifference();
            camera.m_CameraMouseFactor += glm::dvec2(-diff.y * (dt), diff.x * (dt));
            thisBody.rotate(camera.m_CameraMouseFactor.x, camera.m_CameraMouseFactor.y, 0.0);
            camera.m_CameraMouseFactor *= 0.0;
        }
        auto& targetEntity = camera.m_Target;
        EntityDataRequest dataRequest1(targetEntity->entity());

        auto& targetBody = *targetEntity->getComponent<ComponentBody>(dataRequest1);
        auto& targetModel = *targetEntity->getComponent<ComponentModel>(dataRequest1);

        const glm::vec3 pos = (glm::vec3(0.0f, 0.0f, 1.0f) * glm::length(targetModel.radius()) * 0.37f) + (glm::vec3(0.0f, 0.0f, 1.0f) * glm::length(targetModel.radius() * (1.0f + camera.m_OrbitRadius)));

        glm::mat4 cameraModel = glm::mat4(1.0f);
        cameraModel = glm::translate(cameraModel, glm::vec3(targetBody.position()));
        cameraModel *= glm::mat4_cast(glm::quat(thisBody.rotation()));
        cameraModel = glm::translate(cameraModel, pos);

        const glm::vec3 eye(cameraModel[3][0], cameraModel[3][1], cameraModel[3][2]);
        thisBody.setPosition(eye);

        thisCamera.lookAt(eye, targetBody.position(), thisBody.up());
    }
}};

ServerLobbyShipSelectorWindow::ServerLobbyShipSelectorWindow(Core& core,Scene& scene, Camera& camera, const Font& font, const float x, const float y):m_Core(core),m_Font(const_cast<Font&>(font)) {
    m_Width = 578.0f;
    m_Height = 270.0f;
    m_IsCurrentlyDragging = false;
    m_IsCurrentlyOverShipWindow = false;
    auto& logic = *camera.entity().getComponent<ComponentLogic2>();
    logic.setUserPointer1(&core);
    logic.setUserPointer2(this);
    logic.setFunctor(GameCameraShipSelectorLogicFunctor());
    m_ShipWindow = new ScrollFrame(x, y, m_Width, m_Height);
    m_ShipWindow->setContentPadding(0.0f);
    m_ChosenShipName = "";
    
    m_ShipDisplay = &scene.addViewport(x + m_ShipWindow->width() + 3.0f, y - m_Height, m_Height - 1.0f, m_Height, camera);
    //m_ShipDisplay->setDepthMaskValue(100.0f);
    //m_ShipDisplay->activateDepthMask(true);
    m_ShipDisplay->removeRenderFlag(ViewportRenderingFlag::API2D);
    m_ShipDisplay->setAspectRatioSynced(true);

    auto& thisBody = *camera.entity().getComponent<ComponentBody>();
    auto& thisCamera = *camera.entity().getComponent<ComponentCamera>();
    thisBody.rotate(0.0f, glm::radians(190.0f), 0.0f, true);
    thisBody.rotate(glm::radians(10.0f), 0.0f, 0.0f, true);

    GameCamera& gameCamera = *static_cast<GameCamera*>(logic.getUserPointer());
    auto& targetEntity = gameCamera.m_Target;
    auto& targetBody = *targetEntity->getComponent<ComponentBody>();
    auto& targetModel = *targetEntity->getComponent<ComponentModel>();


    const glm::vec3 pos = (glm::vec3(0, 0, 1) * glm::length(targetModel.radius()) * 0.37f) + (glm::vec3(0.0f, 0.0f, 1.0f) * glm::length(targetModel.radius() * (1.0f + gameCamera.m_OrbitRadius)));
    glm::mat4 cameraModel = glm::mat4(1.0f);
    cameraModel = glm::translate(cameraModel, glm::vec3(targetBody.position()));
    cameraModel *= glm::mat4_cast(glm::quat(thisBody.rotation()));
    cameraModel = glm::translate(cameraModel, pos);
    const glm::vec3 eye(cameraModel[3][0], cameraModel[3][1], cameraModel[3][2]);
    thisBody.setPosition(eye);
    thisCamera.lookAt(eye, targetBody.position(), thisBody.up());


}
ServerLobbyShipSelectorWindow::~ServerLobbyShipSelectorWindow() {
    SAFE_DELETE(m_ShipWindow);
}
void ServerLobbyShipSelectorWindow::setShipViewportActive(const bool& active) {
    m_ShipDisplay->activate(active);
}
void ServerLobbyShipSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ShipWindow->setColor(r, g, b, a);
}
void ServerLobbyShipSelectorWindow::setPosition(const float x, const float y) {
    m_ShipWindow->setPosition(x, y);
    m_ShipDisplay->setViewportDimensions(x + m_ShipWindow->width() + 3.0f, y - m_Height, m_Height - 1.0f, m_Height);
}
void ServerLobbyShipSelectorWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void* ServerLobbyShipSelectorWindow::getUserPointer() {
    return m_UserPointer;
}
void ServerLobbyShipSelectorWindow::clear() {
    vector_clear(m_ShipWindow->content());
}
void ServerLobbyShipSelectorWindow::addContent(Widget* widget) {
    m_ShipWindow->addContent(widget);
}
Font& ServerLobbyShipSelectorWindow::getFont() {
    return m_Font;
}
ScrollFrame& ServerLobbyShipSelectorWindow::getWindowFrame() {
    return *m_ShipWindow;
}
const Viewport& ServerLobbyShipSelectorWindow::getShipDisplay() const {
    return *m_ShipDisplay;
}
void ServerLobbyShipSelectorWindow::update(const double& dt) {
    m_ShipWindow->update(dt);
    auto& dimensions = m_ShipDisplay->getViewportDimensions();
    auto& mouse = Engine::getMousePosition();
    m_IsCurrentlyOverShipWindow = true;
    if (mouse.x < dimensions.x || mouse.x > dimensions.x + dimensions.z || mouse.y < dimensions.y || mouse.y > dimensions.y + dimensions.w) {
        m_IsCurrentlyOverShipWindow = false;
    }
    if (!Engine::isMouseButtonDown(MouseButton::Left)) {
        m_IsCurrentlyDragging = false;
    }
    if (m_IsCurrentlyOverShipWindow && Engine::isMouseButtonDown(MouseButton::Left)) {
        m_IsCurrentlyDragging = true;
    }

}
void ServerLobbyShipSelectorWindow::render() {
    m_ShipWindow->render();
    auto& dimensions = m_ShipDisplay->getViewportDimensions();
    Renderer::renderBorder(1.0f, glm::vec2(dimensions.x + 1.0f, dimensions.y), m_ShipWindow->color(), dimensions.z + 1.0f, dimensions.w, 0, 0.01f, Alignment::BottomLeft);
}
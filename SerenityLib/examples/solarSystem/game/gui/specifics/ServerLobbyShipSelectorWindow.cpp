#include "ServerLobbyShipSelectorWindow.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>

#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>

#include <iostream>

#include "../Text.h"

#include "../../Client.h"
#include "../../Core.h"
#include "../../Packet.h"

using namespace Engine;
using namespace std;

ServerLobbyShipSelectorWindow::ServerLobbyShipSelectorWindow(const Scene& scene, const Camera& camera, const Font& font, const float& x, const float& y):m_Font(const_cast<Font&>(font)) {
    m_Width = 583;
    m_Height = 270;

    m_ShipWindow = new ScrollFrame(x, y, m_Width, m_Height);
    m_ShipWindow->setContentPadding(0.0f);
    
    m_ShipDisplay = new Viewport(scene, camera);
    m_ShipDisplay->setDepthMaskValue(100.0f);
    m_ShipDisplay->activateDepthMask();
    m_ShipDisplay->deactivate2DAPI();

    m_ShipDisplay->setViewportDimensions(x + m_ShipWindow->width(), y, m_Height, m_Height);
}
ServerLobbyShipSelectorWindow::~ServerLobbyShipSelectorWindow() {
    SAFE_DELETE(m_ShipWindow);
}

void ServerLobbyShipSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ShipWindow->setColor(r, g, b, a);
}
void ServerLobbyShipSelectorWindow::setPosition(const float& x, const float& y) {
    m_ShipWindow->setPosition(x, y);
    m_ShipDisplay->setViewportDimensions(x + m_ShipWindow->width(), y, m_Height, m_Height);
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
    m_ShipDisplay->activate(); //probably redundant
}
void ServerLobbyShipSelectorWindow::render() {
    m_ShipWindow->render();
    auto& dimensions = m_ShipDisplay->getViewportDimensions();
    Renderer::renderBorder(1, glm::vec2(dimensions.x, dimensions.y), m_ShipWindow->color(), dimensions.z, dimensions.w, 0, 0.01f, Alignment::TopLeft);
}
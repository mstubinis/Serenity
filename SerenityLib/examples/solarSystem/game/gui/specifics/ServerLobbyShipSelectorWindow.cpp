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
#include "../../ships/Ships.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

using namespace Engine;
using namespace std;

ServerLobbyShipSelectorWindow::ServerLobbyShipSelectorWindow(Core& core,Scene& scene, Camera& camera, const Font& font, const float x, const float y):m_Core(core),m_Font(const_cast<Font&>(font)) {
    m_Width = 578.0f;
    m_Height = 270.0f;
    m_ShipWindow = new ScrollFrame(x, y, m_Width, m_Height);
    m_ShipWindow->setContentPadding(0.0f);  
    m_3DViewer = new Ship3DViewer(core, scene, camera, x + m_ShipWindow->width() + 3.0f, y - m_Height, m_Height - 1.0f, m_Height);
}
ServerLobbyShipSelectorWindow::~ServerLobbyShipSelectorWindow() {
    SAFE_DELETE(m_ShipWindow);
    SAFE_DELETE(m_3DViewer);
}
void ServerLobbyShipSelectorWindow::setShipClass(const string& ship_class) {
    m_3DViewer->setShipClass(ship_class);
}
void ServerLobbyShipSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ShipWindow->setColor(r, g, b, a);
}
void ServerLobbyShipSelectorWindow::setPosition(const float x, const float y) {
    m_ShipWindow->setPosition(x, y);
    m_3DViewer->setPosition(x + m_ShipWindow->width() + 3.0f, y - m_Height);
}
const string& ServerLobbyShipSelectorWindow::getShipClass() const {
    return m_3DViewer->getShipClass();
}
void ServerLobbyShipSelectorWindow::setShipViewportActive(const bool& active) {
    m_3DViewer->setShipViewportActive(active);
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
void ServerLobbyShipSelectorWindow::update(const double& dt) {
    m_ShipWindow->update(dt);
    m_3DViewer->update(dt);
}
void ServerLobbyShipSelectorWindow::render() {
    m_ShipWindow->render();
    m_3DViewer->render();
}
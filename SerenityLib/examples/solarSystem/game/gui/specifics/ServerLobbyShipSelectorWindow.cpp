#include "ServerLobbyShipSelectorWindow.h"
#include <core/engine/system/Engine.h>
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

struct ShipSelectorButtonOnClick final { void operator()(Button* button) const {
    ServerLobbyShipSelectorWindow& window = *static_cast<ServerLobbyShipSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.1f, 0.1f, 0.1f, 0.5f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    const string& shipClass = button->text();
    window.setShipClass(shipClass);
}};

ServerLobbyShipSelectorWindow::ServerLobbyShipSelectorWindow(Core& core,Scene& menu_scene, Camera& game_camera, const Font& font, const float x, const float y, const float w, const float h) : m_Core(core), m_Font(const_cast<Font&>(font)) {
    m_ShipWindow = NEW ScrollFrame(x, y, w, h);
    m_ShipWindow->setContentPadding(0.0f);  
    m_3DViewer = NEW Ship3DViewer(core, menu_scene, game_camera, x + m_ShipWindow->width() + 3.0f, y - h, h - 1.0f, h);
}
ServerLobbyShipSelectorWindow::~ServerLobbyShipSelectorWindow() {
    SAFE_DELETE(m_ShipWindow);
    SAFE_DELETE(m_3DViewer);
}
void ServerLobbyShipSelectorWindow::addShipButton(const string& shipClass) {
    auto& textColor = Ships::Database.at(shipClass).FactionInformation.ColorText;
    Button& shipbutton = *(NEW Button(m_Font, 0, 0, 100, 40));
    shipbutton.setText(shipClass);
    shipbutton.setColor(0.1f, 0.1f, 0.1f, 0.5f);
    shipbutton.setTextColor(textColor.r, textColor.g, textColor.b, 1.0f);
    shipbutton.setAlignment(Alignment::TopLeft);
    shipbutton.setTextAlignment(TextAlignment::Left);
    shipbutton.setUserPointer(this);
    shipbutton.setOnClickFunctor(ShipSelectorButtonOnClick());
    shipbutton.setTextureCorner(nullptr);
    shipbutton.setTextureEdge(nullptr);
    shipbutton.setWidth(m_ShipWindow->width());
    addContent(&shipbutton);
}
void ServerLobbyShipSelectorWindow::setShipClass(const string& ship_class) {
    m_3DViewer->setShipClass(ship_class);
}
void ServerLobbyShipSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ShipWindow->setColor(r, g, b, a);
}
void ServerLobbyShipSelectorWindow::setPosition(const float x, const float y) {
    m_ShipWindow->setPosition(x, y);
    m_3DViewer->setPosition(x + m_ShipWindow->width() + 3.0f, y - m_ShipWindow->height());
}
const string& ServerLobbyShipSelectorWindow::getShipClass() const {
    return m_3DViewer->getShipClass();
}
void ServerLobbyShipSelectorWindow::setShipViewportActive(const bool& active) {
    m_3DViewer->setShipViewportActive(active);
}
void ServerLobbyShipSelectorWindow::clear() {
    auto& content = m_ShipWindow->content();
    SAFE_DELETE_VECTOR(content);
    content.clear();
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
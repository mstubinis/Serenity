#include "ServerLobbyConnectedPlayersWindow.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>

#include <iostream>

#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../Core.h"

using namespace std;

ServerLobbyConnectedPlayersWindow::ServerLobbyConnectedPlayersWindow(const Font& font, const float x, const float y) :m_Font(const_cast<Font&>(font)) {
    m_UserPointer = nullptr;
    m_Width = 200.0f;
    m_Height = 300.0f;

    m_ChatWindow = NEW ScrollFrame(font, x, y, m_Width, m_Height);
    m_ChatWindow->setColor(1.0f, 1.0f, 0.0f, 1.0f);
}
ServerLobbyConnectedPlayersWindow::~ServerLobbyConnectedPlayersWindow() {
    SAFE_DELETE(m_ChatWindow);
}
void ServerLobbyConnectedPlayersWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void ServerLobbyConnectedPlayersWindow::addContent(Widget* widget) {
    m_ChatWindow->addContent(widget);
}

const bool ServerLobbyConnectedPlayersWindow::removeContent(const string& key) {
    auto& content = m_ChatWindow->content();
    for (auto& widget : content) {
        Text* text = dynamic_cast<Text*>(widget);
        if (text) {
            if (text->text() == key) {
                m_ChatWindow->removeContent(widget);
                return true;
            }
        }
    }
    return false;
}
void ServerLobbyConnectedPlayersWindow::clear() {
    auto& content = m_ChatWindow->content();
    SAFE_DELETE_VECTOR(content);
    content.clear();
}
void* ServerLobbyConnectedPlayersWindow::getUserPointer() {
    return m_UserPointer;
}
Font& ServerLobbyConnectedPlayersWindow::getFont() {
    return m_Font;
}
ScrollFrame& ServerLobbyConnectedPlayersWindow::getWindowFrame() {
    return *m_ChatWindow;
}
void ServerLobbyConnectedPlayersWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ChatWindow->setColor(r, g, b, a);
}
void ServerLobbyConnectedPlayersWindow::setPosition(const float x, const float y) {
    m_ChatWindow->setPosition(x, y);
}
void ServerLobbyConnectedPlayersWindow::update(const double& dt) {
    m_ChatWindow->update(dt);
}
void ServerLobbyConnectedPlayersWindow::render() {
    m_ChatWindow->render();
}
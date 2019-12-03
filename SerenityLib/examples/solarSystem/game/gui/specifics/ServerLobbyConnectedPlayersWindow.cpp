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

    m_ChatWindow = new ScrollFrame(x, y, m_Width, m_Height);
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

void ServerLobbyConnectedPlayersWindow::removeContent(const string& key) {
    auto& vec = m_ChatWindow->content();
    for (auto& content : vec) {
        Text* text = dynamic_cast<Text*>(content);
        if (text) {
            if (text->text() == key) {
                removeFromVector(vec, content);
            }
        }
    }
    m_ChatWindow->internalAddContent();
}
void ServerLobbyConnectedPlayersWindow::clear() {
    vector_clear(m_ChatWindow->content());
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
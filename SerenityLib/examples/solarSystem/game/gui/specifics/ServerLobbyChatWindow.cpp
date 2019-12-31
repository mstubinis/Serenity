#include "ServerLobbyChatWindow.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>

#include <iostream>

#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../Core.h"
#include "../../networking/packets/PacketMessage.h"

using namespace std;

struct OnEnterFunctor final { void operator()(TextBox* textBox) const {
    ServerLobbyChatWindow& window = *static_cast<ServerLobbyChatWindow*>(textBox->getUserPointer());
    const auto message = textBox->text();
    if (!message.empty()) {
        textBox->setText("");
        auto& scrollFrame = window.getWindowFrame();
        Client& client    = *static_cast<Client*>(window.getUserPointer());
        PacketMessage p;
        p.PacketType      = PacketType::Client_To_Server_Chat_Message;
        p.data            = message;
        p.name            = client.username(); 
        client.send(p);
    }
}};


ServerLobbyChatWindow::ServerLobbyChatWindow(const Font& font, const float x, const float y) : m_Font(const_cast<Font&>(font)){
    m_UserPointer = nullptr;
    m_Width       = 630.0f;
    m_Height      = 300.0f;

    m_ChatWindow  = NEW ScrollFrame(font, x, y, m_Width, m_Height, 0.02f);
    m_ChatWindow->setColor(1.0f, 1.0f, 0.0f, 1.0f);

    m_ChatInput   = NEW TextBox("Chat", font, 85, 0.0f , 0.0f);
    m_ChatInput->setPosition(x + (m_Width / 2.0f) + 145.0f, y - m_Height - m_ChatInput->getTextHeight() / 2.0f - 15.0f);
    m_ChatInput->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ChatInput->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ChatInput->setTextScale(0.62f, 0.62f);
    m_ChatInput->setOnEnterFunctor(OnEnterFunctor());
    m_ChatInput->setUserPointer(this);
}
ServerLobbyChatWindow::~ServerLobbyChatWindow() {
    SAFE_DELETE(m_ChatInput);
    SAFE_DELETE(m_ChatWindow);
}
void ServerLobbyChatWindow::clear() {
    auto& content = m_ChatWindow->content();
    for (auto& widgetEntry : content) {
        SAFE_DELETE(widgetEntry.widget);
    }
    content.clear();
}
void ServerLobbyChatWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void ServerLobbyChatWindow::addContent(Widget* widget) {
    m_ChatWindow->addContent(widget);
}
void* ServerLobbyChatWindow::getUserPointer() {
    return m_UserPointer;
}

Font& ServerLobbyChatWindow::getFont() {
    return m_Font;
}
ScrollFrame& ServerLobbyChatWindow::getWindowFrame() {
    return *m_ChatWindow;
}
TextBox& ServerLobbyChatWindow::getInputFrame() {
    return *m_ChatInput;
}

void ServerLobbyChatWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ChatWindow->setColor(r,g,b,a);
    m_ChatInput->setTextColor(r,g,b,a);
}
void ServerLobbyChatWindow::setPosition(const float x, const float y) {
    m_ChatWindow->setPosition(x, y);
    m_ChatInput->setPosition(x + (m_Width / 2.0f) + 145.0f, y - m_Height - m_ChatInput->getTextHeight() / 2.0f - 15.0f);
}

void ServerLobbyChatWindow::update(const double& dt) {
    m_ChatWindow->update(dt);
    m_ChatInput->update(dt);
}
void ServerLobbyChatWindow::render() {
    m_ChatWindow->render();
    m_ChatInput->render();
}
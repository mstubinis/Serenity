#include "ServerHostingMapSelectorWindow.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>

#include <iostream>
#include <boost/filesystem.hpp>

#include "../Text.h"

#include "../../Client.h"
#include "../../Core.h"
#include "../../Packet.h"
#include "../Button.h"
#include "../ScrollWindow.h"
#include "../../ResourceManifest.h"
#include "../Widget.h"

using namespace Engine;
using namespace std;


struct MapSelectorButtonOnClick final{ void operator()(Button* button) const {
    ServerHostingMapSelectorWindow& window = *static_cast<ServerHostingMapSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.5f, 0.5f, 0.5f, 0.0f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    window.m_CurrentChoice->setText(button->text());
}};


ServerHostingMapSelectorWindow::ServerHostingMapSelectorWindow(const Font& font, const unsigned int& x, const unsigned int& y):m_Font(const_cast<Font&>(font)){
    m_Width = 600;
    m_Height = 300;
    m_MapFileWindow = new ScrollFrame(Resources::getWindowSize().x / 2 - (m_Width / 2), 630, m_Width, m_Height);
    m_MapFileWindow->setColor(1, 1, 0, 1);
    m_MapFileWindow->setContentPadding(0);
    //m_ServerHostMapSelector->setAlignment(Alignment::Center);
    //get a list of maps and add em
    string path = (ResourceManifest::BasePath + "data/Systems/");
    if (!path.empty()) {
        namespace fs = boost::filesystem;
        fs::path apk_path(path);
        fs::recursive_directory_iterator end;
        for (fs::recursive_directory_iterator i(apk_path); i != end; ++i) {
            fs::path cp = (*i);
            Button* button = new Button(m_Font, 0, 0, 100, 40);
            const string& file = (cp.filename().string());
            const string& ext = boost::filesystem::extension(file);
            string copy = file;
            copy = copy.substr(0, copy.size() - ext.size());
            button->setText(copy);
            button->setColor(0.5f, 0.5f, 0.5f, 0.0f);
            button->setTextColor(1, 1, 0, 1);
            button->setAlignment(Alignment::TopLeft);
            button->setWidth(600);
            button->setTextAlignment(TextAlignment::Left);
            button->setUserPointer(this);
            button->setOnClickFunctor(MapSelectorButtonOnClick());

            m_MapFileWindow->addContent(button);
        }
    }
    m_Label = new Text(x, y, m_Font, "Choose Map");
    m_Label->setColor(1, 1, 0, 1);
    const auto& lineHeight = m_Font.getTextHeight("X") * m_Label->textScale().y;
    m_Label->setPosition(x, y + 50);

    m_CurrentChoice = new Text(x + m_Width, y + 50, m_Font);
    m_CurrentChoice->setColor(0, 1, 0, 1);
    m_CurrentChoice->setTextAlignment(TextAlignment::Right);
}
ServerHostingMapSelectorWindow::~ServerHostingMapSelectorWindow() {
    SAFE_DELETE(m_MapFileWindow);
    SAFE_DELETE(m_Label);
    SAFE_DELETE(m_CurrentChoice);
}
const Text& ServerHostingMapSelectorWindow::getCurrentChoice() const {
    return *m_CurrentChoice;
}
void ServerHostingMapSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_MapFileWindow->setColor(r, g, b, a);
}
void ServerHostingMapSelectorWindow::setPosition(const unsigned int& x, const unsigned int& y) {
    m_MapFileWindow->setPosition(x, y);

    const auto& lineHeight = m_Font.getTextHeight("X");
    m_Label->setPosition(x, y + 50);
    m_CurrentChoice->setPosition(x + m_Width, y + 50);
}

void ServerHostingMapSelectorWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void* ServerHostingMapSelectorWindow::getUserPointer() {
    return m_UserPointer;
}
void ServerHostingMapSelectorWindow::clear() {
    vector_clear(m_MapFileWindow->content());
}

void ServerHostingMapSelectorWindow::addContent(Widget* widget) {
    m_MapFileWindow->addContent(widget);
}

Font& ServerHostingMapSelectorWindow::getFont() {
    return m_Font;
}
ScrollFrame& ServerHostingMapSelectorWindow::getWindowFrame() {
    return *m_MapFileWindow;
}

void ServerHostingMapSelectorWindow::update(const double& dt) {
    m_MapFileWindow->update(dt);
    m_Label->update(dt);
    m_CurrentChoice->update(dt);
}
void ServerHostingMapSelectorWindow::render() {
    m_MapFileWindow->render();
    m_Label->render();
    m_CurrentChoice->render();
}
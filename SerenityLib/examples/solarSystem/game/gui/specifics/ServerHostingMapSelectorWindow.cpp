#include "ServerHostingMapSelectorWindow.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>

#include <iostream>
#include <boost/filesystem.hpp>

#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../Core.h"
#include "../Button.h"
#include "../ScrollWindow.h"
#include "../../ResourceManifest.h"
#include "../Widget.h"

using namespace Engine;
using namespace std;


struct MapSelectorButtonOnClick final{ void operator()(Button* button) const {
    ServerHostingMapSelectorWindow& window = *static_cast<ServerHostingMapSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.1f, 0.1f, 0.1f, 0.5f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    window.m_CurrentChoice = button->text();
}};


ServerHostingMapSelectorWindow::ServerHostingMapSelectorWindow(const Font& font, const float x, const float y, const float w, const float h):m_Font(const_cast<Font&>(font)){
    m_UserPointer = nullptr;

    m_MapFileScrollFrame = NEW ScrollFrame(x, y, w, h);
    m_MapFileScrollFrame->setColor(1, 1, 0, 1);
    m_MapFileScrollFrame->setContentPadding(0.0f);

    string path = (ResourceManifest::BasePath + "data/Systems/");
    if (!path.empty()) {
        boost::filesystem::path apk_path(path);
        boost::filesystem::recursive_directory_iterator end;
        for (boost::filesystem::recursive_directory_iterator i(apk_path); i != end; ++i) {
            boost::filesystem::path cp = (*i);
            Button* button             = NEW Button(m_Font, 0.0f, 0.0f, 100.0f, 40.0f);
            const string file          = (cp.filename().string());
            const string ext           = boost::filesystem::extension(file);
            string copy                = file;
            copy                       = copy.substr(0, copy.size() - ext.size());
            button->setText(copy);
            button->setColor(0.1f, 0.1f, 0.1f, 0.5f);
            button->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
            button->setAlignment(Alignment::TopLeft);
            button->setTextAlignment(TextAlignment::Left);
            button->setUserPointer(this);
            button->setOnClickFunctor(MapSelectorButtonOnClick());
            button->setTextureCorner(nullptr);
            button->setTextureEdge(nullptr);
            const auto width_ = m_MapFileScrollFrame->width();
            button->setWidth(width_);

            m_MapFileScrollFrame->addContent(button);
        }
    }
}
ServerHostingMapSelectorWindow::~ServerHostingMapSelectorWindow() {
    SAFE_DELETE(m_MapFileScrollFrame);
}
const string& ServerHostingMapSelectorWindow::getCurrentChoice() const {
    return m_CurrentChoice;
}
void ServerHostingMapSelectorWindow::setSize(const float& w, const float& h) {
    m_MapFileScrollFrame->setSize(w, h);
    for (auto& content : m_MapFileScrollFrame->content()) {
        content->setWidth(m_MapFileScrollFrame->width());
    }
}
void ServerHostingMapSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_MapFileScrollFrame->setColor(r, g, b, a);
}
void ServerHostingMapSelectorWindow::setPosition(const float x, const float y) {
    m_MapFileScrollFrame->setPosition(x, y);

    const auto lineHeight = m_Font.getTextHeight("X");
}
void ServerHostingMapSelectorWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void* ServerHostingMapSelectorWindow::getUserPointer() {
    return m_UserPointer;
}
void ServerHostingMapSelectorWindow::clear() {
    auto& content = m_MapFileScrollFrame->content();
    SAFE_DELETE_VECTOR(content);
    content.clear();
}
void ServerHostingMapSelectorWindow::addContent(Widget* widget) {
    m_MapFileScrollFrame->addContent(widget);
}
Font& ServerHostingMapSelectorWindow::getFont() {
    return m_Font;
}
ScrollFrame& ServerHostingMapSelectorWindow::getWindowFrame() {
    return *m_MapFileScrollFrame;
}
void ServerHostingMapSelectorWindow::update(const double& dt) {
    m_MapFileScrollFrame->update(dt);
}
void ServerHostingMapSelectorWindow::render() {
    m_MapFileScrollFrame->render();
}
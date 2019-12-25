#include "MapDescriptionWindow.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <iostream>

#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../Core.h"
#include "../Button.h"
#include "../ScrollWindow.h"
#include "../../ResourceManifest.h"
#include "../Widget.h"
#include "../../Helper.h"

using namespace std;

MapDescriptionWindow::MapDescriptionWindow(Font& font, const float& x, const float& y, const float& width, const float& height) : m_Font(font){
    m_UserPointer = nullptr;
    m_ScrollFrame = NEW ScrollFrame(font, x, y, width, height);
    m_ScrollFrame->setColor(1, 1, 0, 1);
    m_ScrollFrame->setContentPadding(30.0f);
}
MapDescriptionWindow::~MapDescriptionWindow() {
    SAFE_DELETE(m_ScrollFrame);
}
void MapDescriptionWindow::setCurrentMapChoice(const MapEntryData& choice) {
    //0 - map name
    //1 - map file path
    //2 - map description
    clear();
    Text* text = NEW Text(0, 0, m_Font, choice.map_desc);
    text->setColor(1, 1, 0, 1);
    text->setTextScale(0.85f,0.85f);
    addContent(text);
}
void MapDescriptionWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ScrollFrame->setColor(r, g, b, a);
}
void MapDescriptionWindow::setPosition(const float x, const float y) {
    m_ScrollFrame->setPosition(x, y);
}

void MapDescriptionWindow::onResize(const unsigned int newWidth, const unsigned int newHeight) {

}

void MapDescriptionWindow::setSize(const float& w, const float& h) {
    m_ScrollFrame->setSize(w, h);
}

void MapDescriptionWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void* MapDescriptionWindow::getUserPointer() {
    return m_UserPointer;
}
void MapDescriptionWindow::clear() {
    m_ScrollFrame->clear();
}

void MapDescriptionWindow::addContent(Widget* widget) {
    m_ScrollFrame->addContent(widget);
}

Font& MapDescriptionWindow::getFont() {
    return m_Font;
}
ScrollFrame& MapDescriptionWindow::getWindowFrame() {
    return *m_ScrollFrame;
}

void MapDescriptionWindow::update(const double& dt) {
    m_ScrollFrame->update(dt);
}
void MapDescriptionWindow::render() {
    m_ScrollFrame->render();
}
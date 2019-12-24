#include "HostScreen.h"
#include "ServerHostingMapSelectorWindow.h"
#include "MapDescriptionWindow.h"
#include "../../map/Map.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../Core.h"
#include "../Button.h"
#include "../ScrollWindow.h"
#include "../../ResourceManifest.h"
#include "../Widget.h"
#include "../../Helper.h"

using namespace Engine;
using namespace std;
namespace boost_io = boost::iostreams;

const auto bottom_bar_height = 50.0f;

struct MapSelectorButtonOnClick final{ void operator()(Button* button) const {
    auto& button_data = *static_cast<ServerHostingMapSelectorWindow::ButtonPtr*>(button->getUserPointer());
    auto& map_selection_window = button_data.hostScreen->getMapSelectionWindow();
    auto& map_desc_window = button_data.hostScreen->getMapDescriptionWindow();
    map_selection_window.clear_chosen_map();
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    map_selection_window.m_CurrentChoice = button_data.mapChoice;

    map_desc_window.setCurrentMapChoice(button_data.mapChoice);
}};

struct CycleGameModeLeftButtonOnClick final { void operator()(Button* button) const {
    auto& window = *static_cast<ServerHostingMapSelectorWindow*>(button->getUserPointer());
    window.clear_chosen_map();
    window.clear();
    int num = static_cast<int>(window.m_CurrentGameMode);
    --num;
    if (num < 0) {
        num = GameplayModeType::_TOTAL - 1;
    }
    window.m_CurrentGameMode = static_cast<GameplayModeType::Mode>(num);
    window.recalculate_maps();
}};
struct CycleGameModeRightButtonOnClick final { void operator()(Button* button) const {
    auto& window = *static_cast<ServerHostingMapSelectorWindow*>(button->getUserPointer());
    window.clear_chosen_map();
    window.clear();
    int num = static_cast<int>(window.m_CurrentGameMode);
    ++num;
    if (num >= GameplayModeType::_TOTAL) {
        num = 0;
    }
    window.m_CurrentGameMode = static_cast<GameplayModeType::Mode>(num);
    window.recalculate_maps();
}};


ServerHostingMapSelectorWindow::ServerHostingMapSelectorWindow(HostScreen& host, const Font& font, const float x, const float y, const float w, const float h) : m_HostScreen(host), m_Font(const_cast<Font&>(font)){
    const auto content_window_size = glm::vec2(w,h) - glm::vec2(0, bottom_bar_height);

    m_UserPointer        = nullptr;
    m_CurrentGameMode    = GameplayModeType::FFA;

    m_MapFileScrollFrame = NEW ScrollFrame(x, y, content_window_size.x, content_window_size.y);
    m_MapFileScrollFrame->setColor(1, 1, 0, 1);
    m_MapFileScrollFrame->setContentPadding(0.0f);

    auto& scroll_frame_pos = m_MapFileScrollFrame->positionWorld();

    m_ChangeGameModeLeftButton = NEW Button(font, glm::vec2(scroll_frame_pos.x + (bottom_bar_height / 2.0f), scroll_frame_pos.y - m_MapFileScrollFrame->height() - (bottom_bar_height / 2.0f)), bottom_bar_height, bottom_bar_height);
    m_ChangeGameModeLeftButton->setText("<");
    m_ChangeGameModeLeftButton->setAlignment(Alignment::TopLeft);
    m_ChangeGameModeLeftButton->setDepth(0.054f);
    m_ChangeGameModeLeftButton->setUserPointer(this);
    m_ChangeGameModeLeftButton->setOnClickFunctor(CycleGameModeLeftButtonOnClick());
    m_ChangeGameModeLeftButton->setColor(0.5f, 0.78f, 0.94f, 1.0f);
    m_ChangeGameModeLeftButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_ChangeGameModeRightButton = NEW Button(font, glm::vec2((scroll_frame_pos.x + m_MapFileScrollFrame->width())- (bottom_bar_height / 2.0f), scroll_frame_pos.y - m_MapFileScrollFrame->height() - (bottom_bar_height / 2.0f)), bottom_bar_height, bottom_bar_height);
    m_ChangeGameModeRightButton->setText(">");
    m_ChangeGameModeRightButton->setAlignment(Alignment::TopRight);
    m_ChangeGameModeRightButton->setDepth(0.054f);
    m_ChangeGameModeRightButton->setUserPointer(this);
    m_ChangeGameModeRightButton->setOnClickFunctor(CycleGameModeRightButtonOnClick());
    m_ChangeGameModeRightButton->setColor(0.5f, 0.78f, 0.94f, 1.0f);
    m_ChangeGameModeRightButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);

    recalculate_maps();
}
ServerHostingMapSelectorWindow::~ServerHostingMapSelectorWindow() {
    for (auto& widget : m_MapFileScrollFrame->content()) {
        auto* map_ptr_data = static_cast<ServerHostingMapSelectorWindow::ButtonPtr*>(widget->getUserPointer());
        SAFE_DELETE(map_ptr_data);
    }
    SAFE_DELETE(m_MapFileScrollFrame);
    SAFE_DELETE(m_ChangeGameModeLeftButton);
    SAFE_DELETE(m_ChangeGameModeRightButton);
}
void ServerHostingMapSelectorWindow::clear_chosen_map() {
    m_CurrentChoice = MapEntryData();
    for (auto& widget : m_MapFileScrollFrame->content()) {
        widget->setColor(0.1f, 0.1f, 0.1f, 0.5f);
    }
}
void ServerHostingMapSelectorWindow::recalculate_maps() {
    for (auto& itr : Map::DATABASE) {     
        auto& data = itr.second;
        for (auto& game_mode_int : data.map_valid_game_modes) {
            if (game_mode_int == static_cast<unsigned int>(m_CurrentGameMode)) {
                Button& button = *NEW Button(m_Font, 0.0f, 0.0f, 100.0f, 40.0f);
                button.setText(data.map_name);
                button.setColor(0.1f, 0.1f, 0.1f, 0.5f);
                button.setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
                button.setAlignment(Alignment::TopLeft);
                button.setTextAlignment(TextAlignment::Left);

                auto* ptr = NEW ServerHostingMapSelectorWindow::ButtonPtr();
                ptr->hostScreen = &m_HostScreen;
                ptr->mapChoice = Map::DATABASE.at(data.map_name);
                button.setUserPointer(ptr);

                button.setOnClickFunctor(MapSelectorButtonOnClick());
                button.setTextureCorner(nullptr);
                button.setTextureEdge(nullptr);
                const auto width_ = m_MapFileScrollFrame->width();
                button.setWidth(width_);

                m_MapFileScrollFrame->addContent(&button);
            }
        }
    }
}
const MapEntryData& ServerHostingMapSelectorWindow::getCurrentChoice() const {
    return m_CurrentChoice;
}
void ServerHostingMapSelectorWindow::setSize(const float& w, const float& h) {
    const auto content_window_size = glm::vec2(w, h) - glm::vec2(0, bottom_bar_height);

    m_MapFileScrollFrame->setSize(content_window_size.x, content_window_size.y);
    for (auto& content : m_MapFileScrollFrame->content()) {
        content->setWidth(m_MapFileScrollFrame->width());
    }

    const auto pos = m_MapFileScrollFrame->position();
    const auto frame_size = glm::vec2(m_MapFileScrollFrame->width(), m_MapFileScrollFrame->height());

    m_ChangeGameModeLeftButton->setPosition(
        pos.x + 2.0f,
        pos.y - frame_size.y - 1.0f
    );
    m_ChangeGameModeRightButton->setPosition(
        pos.x + frame_size.x,
        pos.y - frame_size.y - 1.0f
    );
}
void ServerHostingMapSelectorWindow::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_MapFileScrollFrame->setColor(r, g, b, a);
}
void ServerHostingMapSelectorWindow::onResize(const unsigned int newWidth, const unsigned int newHeight) {

}
void ServerHostingMapSelectorWindow::setPosition(const float x, const float y) {
    m_MapFileScrollFrame->setPosition(x, y);
}
void ServerHostingMapSelectorWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void* ServerHostingMapSelectorWindow::getUserPointer() {
    return m_UserPointer;
}
void ServerHostingMapSelectorWindow::clear() {
    auto& content = m_MapFileScrollFrame->content();
    for (auto& widget : content) {
        auto* map_ptr_data = static_cast<ServerHostingMapSelectorWindow::ButtonPtr*>(widget->getUserPointer());
        SAFE_DELETE(map_ptr_data);
    }
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
    m_ChangeGameModeLeftButton->update(dt);
    m_ChangeGameModeRightButton->update(dt);
}
void ServerHostingMapSelectorWindow::render() {
    m_MapFileScrollFrame->render();
    m_ChangeGameModeLeftButton->render();
    m_ChangeGameModeRightButton->render();

    const auto pos = m_MapFileScrollFrame->position();

    //render bottom bar game mode selection
    Renderer::renderBorder(1, glm::vec2(pos.x + 1, pos.y -( m_MapFileScrollFrame->height() + 2)), glm::vec4(1,1,0,1), m_MapFileScrollFrame->width() + 1, bottom_bar_height, 0.0f, 0.016f, Alignment::TopLeft);
    Renderer::renderText(
        GameplayMode::GAMEPLAY_TYPE_ENUM_NAMES[m_CurrentGameMode],
        m_Font,
        glm::vec2(pos.x + (m_MapFileScrollFrame->width() / 2.0f), pos.y - (m_MapFileScrollFrame->height() + 8) ),
        glm::vec4(1, 1, 0, 1),
        0.0f,
        glm::vec2(1.0f),
        0.0159f,
        TextAlignment::Center
    );

    //render map screenshot

    const auto& current_map_data = m_HostScreen.getMapSelectionWindow().getCurrentChoice();
    const auto map_selector_size = glm::vec2(m_HostScreen.getMapDescriptionWindow().getWindowFrame().width(), m_HostScreen.getMapDescriptionWindow().getWindowFrame().height());
    const auto ss_pos = pos + glm::vec2(m_MapFileScrollFrame->width() + m_HostScreen.getMapDescriptionWindow().getWindowFrame().width(), 0);
    if (!current_map_data.map_name.empty()) {     
        Texture& texture = *(Texture*)current_map_data.map_screenshot_handle.get();
        auto scl = map_selector_size / glm::vec2(texture.size());
        Renderer::renderTexture(texture, ss_pos + glm::vec2(6,0), glm::vec4(1.0f), 0.0f, scl, 0.0141f, Alignment::TopLeft);
    }
    Renderer::renderBorder(1, ss_pos + glm::vec2(5,0), glm::vec4(1, 1, 0, 1), map_selector_size.x + 2, map_selector_size.y, 0.0f, 0.014f, Alignment::TopLeft);
}
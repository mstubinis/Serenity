#include "MapSelectionWindow.h"
#include "MapDescriptionWindow.h"
#include "HostScreen1.h"
#include "../../factions/Faction.h"
#include "../../map/MapDatabase.h"
#include "../../networking/server/Server.h"
#include "../ScrollFrame.h"
#include "../Text.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace Engine;
using namespace std;
namespace boost_io = boost::iostreams;

constexpr auto button_size          = 40.0f;
constexpr auto scroll_frame_padding = 30.0f;

struct MapSelectorButtonOnClick final { void operator()(Button* button) const {
    auto& button_data          = *static_cast<MapSelectionWindow::ButtonPtr*>(button->getUserPointer());
    auto& hostScreen           = *button_data.hostScreen;
    auto& map_selection_window = hostScreen.getMapSelectionWindow();
    auto& map_desc_window      = hostScreen.getMapDescriptionWindow();
    map_selection_window.clear_chosen_map();
    button->setColor(Factions::Database[FactionEnum::Federation].GUIColorHighlight);
    button->disableMouseover();

    Server::SERVER_HOST_DATA.setMapChoice(button_data.mapChoice);
    hostScreen.setCurrentMapChoice(button_data.mapChoice);
}};

struct CycleGameModeLeftButtonOnClick final { void operator()(Button* button) const {
    auto& mapSelectionWindow   = *static_cast<MapSelectionWindow*>(button->getUserPointer());
    auto& map_desc_window      = mapSelectionWindow.m_HostScreen.getMapDescriptionWindow();
    int num = static_cast<int>(Server::SERVER_HOST_DATA.getGameplayModeType());
    --num;
    if (num < 0) {
        num = GameplayModeType::_TOTAL - 1;
    }
    mapSelectionWindow.m_HostScreen.clearCurrentMapChoice();
    mapSelectionWindow.m_HostScreen.setCurrentGameMode(static_cast<GameplayModeType::Mode>(num));
}};
struct CycleGameModeRightButtonOnClick final { void operator()(Button* button) const {
    auto& mapSelectionWindow   = *static_cast<MapSelectionWindow*>(button->getUserPointer());
    auto& map_desc_window      = mapSelectionWindow.m_HostScreen.getMapDescriptionWindow();
    int num = static_cast<int>(Server::SERVER_HOST_DATA.getGameplayModeType());
    ++num;
    if (num >= GameplayModeType::_TOTAL) {
        num = 0;
    }
    mapSelectionWindow.m_HostScreen.clearCurrentMapChoice();
    mapSelectionWindow.m_HostScreen.setCurrentGameMode(static_cast<GameplayModeType::Mode>(num));
}};

MapSelectionWindow::MapSelectionWindow(HostScreen1& hostScreen, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font,x,y,width,height,depth,borderSize,labelText), m_HostScreen(hostScreen), m_Font(font){

    m_ChangeGameModeLeftButton = NEW Button(font, x - (width / 2.0f) - (scroll_frame_padding - 1.0f), y + (height / 2.0f) - 5.0f, button_size, button_size);
    m_ChangeGameModeLeftButton->setText("<");
    m_ChangeGameModeLeftButton->setAlignment(Alignment::TopLeft);
    m_ChangeGameModeLeftButton->setDepth(depth - 0.002f);
    m_ChangeGameModeLeftButton->setUserPointer(this);
    m_ChangeGameModeLeftButton->setOnClickFunctor(CycleGameModeLeftButtonOnClick());
    m_ChangeGameModeLeftButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ChangeGameModeLeftButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    
    m_ChangeGameModeRightButton = NEW Button(font, x + (width / 2.0f) + (scroll_frame_padding - 1.0f), y + (height / 2.0f) - 5.0f, button_size, button_size);
    m_ChangeGameModeRightButton->setText(">");
    m_ChangeGameModeRightButton->setAlignment(Alignment::TopRight);
    m_ChangeGameModeRightButton->setDepth(depth - 0.002f);
    m_ChangeGameModeRightButton->setUserPointer(this);
    m_ChangeGameModeRightButton->setOnClickFunctor(CycleGameModeRightButtonOnClick());
    m_ChangeGameModeRightButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ChangeGameModeRightButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_MapFileScrollFrame = NEW ScrollFrame(font, x, y - (height / 2.0f) + scroll_frame_padding, width - (scroll_frame_padding * 2.0f), height - 110.0f, depth - 0.001f);
    m_MapFileScrollFrame->setAlignment(Alignment::BottomCenter);

    m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
}
MapSelectionWindow::~MapSelectionWindow() {
    clearWindow();
    SAFE_DELETE(m_MapFileScrollFrame);
    SAFE_DELETE(m_ChangeGameModeLeftButton);
    SAFE_DELETE(m_ChangeGameModeRightButton);
}

void MapSelectionWindow::recalculate_maps() {
    clearWindow();
    const auto gameplayMode = static_cast<unsigned int>(Server::SERVER_HOST_DATA.getGameplayModeType());
    for (auto& itr : MapDatabase::DATABASE) {
        auto& data = itr.second;
        for (auto& game_mode_int : data.map_valid_game_modes) {
            if (game_mode_int == gameplayMode) {
                Button& button = *NEW Button(m_Font, 0.0f, 0.0f, m_MapFileScrollFrame->width(), 180.0f);
                button.setText(data.map_name);
                button.setColor(Factions::Database[FactionEnum::Federation].GUIColor);
                button.setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
                button.setAlignment(Alignment::TopLeft);
                button.setTextAlignment(TextAlignment::Left);

                auto* ptr = NEW MapSelectionWindow::ButtonPtr();
                ptr->hostScreen = &m_HostScreen;
                ptr->mapChoice = MapDatabase::DATABASE.at(data.map_name);
                button.setUserPointer(ptr);
                button.setOnClickFunctor(MapSelectorButtonOnClick());
                button.setPaddingSize(40);

                m_MapFileScrollFrame->addContent(&button);
            }
        }
    }
}
void MapSelectionWindow::clear_chosen_map() {
    for (auto& row : m_MapFileScrollFrame->content()) {
        for (auto& widgetEntry : row.widgets) {
            auto& button = *static_cast<Button*>(widgetEntry.widget);
            button.enableMouseover();
            button.setColor(Factions::Database[FactionEnum::Federation].GUIColor);
        }
    }
}
void MapSelectionWindow::clearWindow() {
    auto& content = m_MapFileScrollFrame->content();
    if (content.size() > 0) {
        for (auto& row : content) {
            for (auto& widgetEntry : row.widgets) {
                auto* map_ptr_data = static_cast<MapSelectionWindow::ButtonPtr*>(widgetEntry.widget->getUserPointer());
                SAFE_DELETE(map_ptr_data);
            }
        }
        m_MapFileScrollFrame->clear();
    }
}
void MapSelectionWindow::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    m_MapFileScrollFrame->setPosition(
        pos.x,
        pos.y - (frame_size.y / 2.0f) + scroll_frame_padding
    );
    m_MapFileScrollFrame->setSize(
        frame_size.x - (scroll_frame_padding * 2.0f),
        frame_size.y - 110.0f
    );

    m_ChangeGameModeLeftButton->setPosition(pos.x - (frame_size.x / 2.0f) + (scroll_frame_padding - 1.0f), pos.y + (frame_size.y / 2.0f) - 5.0f);
    m_ChangeGameModeRightButton->setPosition(pos.x + (frame_size.x / 2.0f) - (scroll_frame_padding - 1.0f), pos.y + (frame_size.y / 2.0f) - 5.0f);
}

void MapSelectionWindow::update(const double& dt) {
    m_ChangeGameModeLeftButton->update(dt);
    m_ChangeGameModeRightButton->update(dt);

    RoundedWindow::update(dt);

    m_MapFileScrollFrame->update(dt);
}
void MapSelectionWindow::render() {
    m_ChangeGameModeLeftButton->render();
    m_ChangeGameModeRightButton->render();

    RoundedWindow::render();

    m_MapFileScrollFrame->render();
}
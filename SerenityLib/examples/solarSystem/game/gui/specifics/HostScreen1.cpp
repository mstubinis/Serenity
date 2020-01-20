#include "HostScreen1.h"

#include "HostScreenFFA2.h"
#include "HostScreenTeamDeathmatch2.h"
#include "HostScreenHomelandSecurity2.h"

#include "../RoundedWindow.h"
#include "MapSelectionWindow.h"
#include "MapDescriptionWindow.h"
#include "ServerLobbyChatWindow.h"
#include "../../Menu.h"
#include "../../factions/Faction.h"
#include "../../Core.h"
#include "../Button.h"
#include "../TextBox.h"
#include "../Text.h"
#include "../../modes/GameplayMode.h"

#include "../../networking/client/Client.h"
#include "../../networking/server/Server.h"

#include "../../teams/TeamIncludes.h"
#include "../../teams/Team.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/discord/Discord.h>

#include <regex>

using namespace Engine;
using namespace std;

constexpr auto padding_x               = 100.0f;
constexpr auto padding_y               = 100.0f;
constexpr auto bottom_bar_height       = 50.0f;
constexpr auto bottom_bar_button_width = 150.0f;

constexpr auto bottom_bar_height_total = 80;

constexpr auto left_window_width       = 550;

struct Host_ButtonBack_OnClick { void operator()(Button* button) const {
    HostScreen1& hostScreen = *static_cast<HostScreen1*>(button->getUserPointer());
    auto& menu             = hostScreen.getMenu();
    menu.setGameState(GameState::Host_Screen_Setup_0);
    menu.setErrorText("", 0.2f);
}};
struct Host_ButtonNext_OnClick { void operator()(Button* button) const {
    HostScreen1& hostScreen = *static_cast<HostScreen1*>(button->getUserPointer());
    auto& menu             = hostScreen.getMenu();
    auto& data             = Server::SERVER_HOST_DATA;
    auto& current_map      = data.getMapChoice();
    if (!data.getMapChoice().map_file_path.empty()) {
        auto& core = menu.getCore();
        switch (data.getGameplayModeType()) {
            case GameplayModeType::FFA: {
                menu.m_HostScreenFFA2->setTopText(current_map.map_name + " - " + data.getGameplayModeString());
                menu.setGameState(GameState::Host_Screen_Setup_FFA_2);
                break;
            }case GameplayModeType::TeamDeathmatch: {
                menu.m_HostScreenTeamDeathmatch2->setTopText(current_map.map_name + " - " + data.getGameplayModeString());
                menu.setGameState(GameState::Host_Screen_Setup_TeamDeathMatch_2);
                break;
            }case GameplayModeType::HomelandSecurity: {
                menu.m_HostScreenHomelandSecurity2->setTopText(current_map.map_name + " - " + data.getGameplayModeString());
                menu.setGameState(GameState::Host_Screen_Setup_HomelandSecurity_2);
                break;
            }default: {
                break;
            }
        }
        

        Discord::DiscordActivity activity;
        activity.setDetail(data.getGameplayModeString());
        activity.setType(discord::ActivityType::Playing);
        activity.setInstance(false);
        activity.setState(current_map.map_name);
        activity.setTimestampStart(0);
        activity.setTimestampEnd(0);
        activity.setImageLarge("large_icon");
        activity.setImageLargeText("");
        activity.setImageSmallText("");
        Discord::update_activity(activity);


        menu.setErrorText(""); 
    }else {
        menu.setErrorText("Please choose a map");
    }
}};

HostScreen1::HostScreen1(Menu& menu, Font& font) : m_Menu(menu), m_Font(font){
    const auto winSize                 = glm::vec2(Resources::getWindowSize());
    const auto contentSize             = winSize - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height      = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;

    m_IsPersistent = false;

    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, bottom_bar_height_total / 2.0f, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicBottom->setDepth(0.512f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
    {
        m_LeftWindow = NEW MapSelectionWindow(*this, font,
            (padding_x / 2.0f) + (left_window_width / 2.0f),
            winSize.y - (padding_y / 2.0f) - (window_height / 2.0f),
            left_window_width,
            window_height, 
        0.05f, 1, "Free for All");
        struct LeftSizeFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2(left_window_width, window_height);
        }};
        struct LeftPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            const auto x = (padding_x / 2.0f) + (left_window_width / 2.0f);
            const auto y = winSize.y - (padding_y / 2.0f) - (window_height / 2.0f);
            return glm::vec2(x, y);
        }};
        m_LeftWindow->setPositionFunctor(LeftPositionFunctor());
        m_LeftWindow->setSizeFunctor(LeftSizeFunctor());

    }
    {
        m_RightWindow = NEW MapDescriptionWindow(font,
            winSize.x - (padding_x / 2.0f) - (window_height / 2.0f),
            winSize.y - (padding_y / 2.0f) - (window_height / 2.0f),
            window_height,
            window_height,
        0.035f, 1, "");
        struct RightSizeFunctor {glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2(window_height, window_height);
        }};
        struct RightPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            const auto x = winSize.x - (padding_x / 2.0f) - (window_height / 2.0f);
            const auto y = winSize.y - (padding_y / 2.0f) - (window_height / 2.0f);
            return glm::vec2(x, y);
        }};
        m_RightWindow->setPositionFunctor(RightPositionFunctor());
        m_RightWindow->setSizeFunctor(RightSizeFunctor());
    }

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host_ButtonBack_OnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(Host_ButtonNext_OnClick());

    setCurrentGameMode(GameplayModeType::FFA);
}
HostScreen1::~HostScreen1() {
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_LeftWindow);
    SAFE_DELETE(m_RightWindow);
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
}
void HostScreen1::setPersistent() {
    m_IsPersistent = true;
}
const bool HostScreen1::isPersistent() const {
    return m_IsPersistent;
}
void HostScreen1::clearCurrentMapChoice() {
    Server::SERVER_HOST_DATA.setMapChoice(MapEntryData());
    m_LeftWindow->clear_chosen_map();
    m_RightWindow->clear();
}
void HostScreen1::setCurrentMapChoice(const MapEntryData& choice) {
    m_RightWindow->clear();
    m_RightWindow->setLabelText(choice.map_name);
    Text* text = NEW Text(0, 0, m_Font, choice.map_desc);
    text->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    text->setTextScale(0.85f, 0.85f);
    text->setAlignment(Alignment::TopLeft);
    m_RightWindow->addContent(text);
}
void HostScreen1::setCurrentGameMode(const GameplayModeType::Mode& currentGameMode) {
    Server::SERVER_HOST_DATA.setGameplayModeType(currentGameMode);
    m_LeftWindow->setLabelText(GameplayMode::GAMEPLAY_TYPE_ENUM_NAMES[currentGameMode]);
    m_LeftWindow->recalculate_maps();
}
Menu& HostScreen1::getMenu() {
    return m_Menu;
}
MapSelectionWindow& HostScreen1::getMapSelectionWindow() {
    return *m_LeftWindow;
}
MapDescriptionWindow& HostScreen1::getMapDescriptionWindow() {
    return *m_RightWindow;
}
void HostScreen1::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::uvec2(newWidth, newHeight);

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);

    m_LeftWindow->onResize(newWidth, newHeight);
    m_RightWindow->onResize(newWidth, newHeight); 

}

void HostScreen1::update(const double& dt) {
    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_LeftWindow->update(dt);
    m_RightWindow->update(dt);
    m_BackgroundEdgeGraphicBottom->update(dt);
}
void HostScreen1::render() {
    m_BackButton->render();
    m_ForwardButton->render();

    m_LeftWindow->render();
    m_RightWindow->render();
    m_BackgroundEdgeGraphicBottom->render();
}
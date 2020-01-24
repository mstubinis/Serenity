#include "HostScreenFFA2.h"
#include "FFAShipSelector.h"
#include "FFAServerInfo.h"
#include "LobbyFFA.h"
#include "HostScreen1.h"
#include "MapDescriptionWindow.h"
#include "ServerLobbyChatWindow.h"
#include "../../Menu.h"
#include "../../factions/Faction.h"
#include "../../Core.h"
#include "../Button.h"
#include "../TextBox.h"
#include "../Text.h"
#include "../../Helper.h"
#include "../../config/ConfigFile.h"

#include "../../networking/client/Client.h"
#include "../../networking/server/Server.h"

#include "../../teams/TeamIncludes.h"
#include "../../teams/Team.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/discord/Discord.h>
#include <boost/algorithm/string/replace.hpp>

#include <regex>

using namespace Engine;
using namespace std;

constexpr auto padding_x                 = 100.0f;
constexpr auto padding_y                 = 100.0f;
constexpr auto bottom_bar_height         = 50.0f;
constexpr auto bottom_bar_button_width   = 150.0f;

constexpr auto bottom_bar_height_total   = 80;
constexpr auto top_bar_height_total      = 60;

constexpr auto right_window_width        = 486;

struct Host2FFA_ButtonBack_OnClick { void operator()(Button* button) const {
    auto& hostScreenFFA = *static_cast<HostScreenFFA2*>(button->getUserPointer());

    auto& menu = hostScreenFFA.getMenu();
    menu.setGameState(GameState::Host_Screen_Setup_1);

    //force server to disconnect client
    menu.getCore().shutdownClient();
    menu.getCore().shutdownServer();
}};
struct Host2FFA_ButtonNext_OnClick { void operator()(Button* button) const {
    auto& hostScreenFFA           = *static_cast<HostScreenFFA2*>(button->getUserPointer());
    auto& menu                    = hostScreenFFA.getMenu();
    const auto& username          = hostScreenFFA.m_SetupServerInfoWindow->getYourNameTextBox().text();
    const auto& port_text         = hostScreenFFA.m_SetupServerInfoWindow->getServerPortTextBox().text();
    const auto& max_players_text  = hostScreenFFA.m_SetupServerInfoWindow->getMaxPlayersTextBox().text();
    const auto& map               = Server::SERVER_HOST_DATA.getMapChoice();

    if (hostScreenFFA.validateShipSelector()) {
        if (hostScreenFFA.validateServerPortTextBox()) {
            if (hostScreenFFA.validateUsernameTextBox()) {
                if (hostScreenFFA.validateMatchDurationTextBox()) {
                    if (hostScreenFFA.validateMaxNumPlayersTextBox()) {
                        auto& core            = menu.getCore();
                        auto& data            = Server::SERVER_HOST_DATA;

                        const int port        = stoi(port_text);
                        const int max_players = stoi(max_players_text);

                        core.startServer(port);
                        core.startClient(nullptr, port, username, "127.0.0.1"); //the client will request validation at this stage

                        vector<TeamNumber::Enum> nil;
                        Team team1 = Team(TeamNumber::Team_FFA, nil, nil);

                        Server::SERVER_HOST_DATA.setGameplayModeType(GameplayModeType::FFA);
                        Server::SERVER_HOST_DATA.setMaxAmountOfPlayers(max_players);
                        Server::SERVER_HOST_DATA.addTeam(team1);
                        Server::SERVER_HOST_DATA.setMatchDurationInSeconds(hostScreenFFA.getMatchDurationFromTextBoxInSeconds());
                        Server::SERVER_HOST_DATA.setServerPort(port);
                        Server::SERVER_HOST_DATA.setLobbyTimeInSeconds(hostScreenFFA.getLobbyDurationFromTextBoxInSeconds());
                        Server::SERVER_HOST_DATA.setCurrentLobbyTimeInSeconds(hostScreenFFA.getLobbyDurationFromTextBoxInSeconds());

                        core.getClient()->getGameplayMode() = Server::SERVER_HOST_DATA.getGameplayMode();
                        core.getServer()->startupMap(map);

                        menu.m_ServerLobbyChatWindow->setUserPointer(core.getClient());

                        ConfigFile config;
                        config.updateHostServerName(username);
                        config.updateHostServerPort(port);

                        menu.setErrorText("");


                        Discord::DiscordActivity activity;
                        activity.setDetail(data.getGameplayModeString());
                        activity.setType(discord::ActivityType::Playing);
                        activity.setInstance(false);
                        activity.setState(map.map_name);
                        activity.setTimestampStart(0);
                        activity.setTimestampEnd(0);
                        activity.setImageLarge("large_icon");
                        activity.setImageLargeText("");
                        activity.setImageSmallText("");
                        activity.setPartySizeCurrent(1);
                        activity.setPartySizeMax(max_players);
                        Discord::update_activity(activity);


                        //TODO: start server and client, but do NOT load the map fully, leave that for the end.
                    }
                }
            }
        }
    }
}};


HostScreenFFA2::HostScreenFFA2(HostScreen1& hostScreen1, Menu& menu, Font& font) : m_Menu(menu), m_HostScreen1(hostScreen1) {
    const auto winSize                 = glm::vec2(Resources::getWindowSize());
    const auto contentSize             = winSize - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height      = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;

    m_IsPersistent = false;

    m_BackgroundEdgeGraphicTop = NEW Button(font, winSize.x / 2.0f, winSize.y, winSize.x, top_bar_height_total);
    m_BackgroundEdgeGraphicTop->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicTop->setAlignment(Alignment::TopCenter);
    m_BackgroundEdgeGraphicTop->setDepth(0.2f);
    m_BackgroundEdgeGraphicTop->disable();
    m_BackgroundEdgeGraphicTop->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicTop->enableTextureCorner(false);
    m_TopLabel = NEW Text(winSize.x / 2.0f, winSize.y - (top_bar_height_total / 2.0f) + 15.0f, font);
    m_TopLabel->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_TopLabel->setAlignment(Alignment::Center);
    m_TopLabel->setTextAlignment(TextAlignment::Center);

    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, 0, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicBottom->setAlignment(Alignment::BottomCenter);
    m_BackgroundEdgeGraphicBottom->setDepth(0.2f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host2FFA_ButtonBack_OnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(Host2FFA_ButtonNext_OnClick());

    const auto window_height = (winSize.y - bottom_bar_height_total - padding_y) - top_bar_height_total;
    const auto left_window_width = winSize.x - right_window_width - padding_x - (padding_x / 2.0f);

    {
        m_SetupShipSelectorWindow = NEW FFAShipSelector(*this, font,
            (padding_x / 2.0f) + (left_window_width / 2.0f),
            winSize.y - (padding_y / 2.0f) - (window_height / 2.0f),
            left_window_width,
            window_height,
        0.1f, 1, "");

        struct LeftSizeFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize             = Resources::getWindowSize();
            const auto window_height_2     = winSize.y - bottom_bar_height_total - padding_y - top_bar_height_total;
            const auto left_window_width_2 = winSize.x - right_window_width - padding_x - (padding_x / 2.0f);
            return glm::vec2(left_window_width_2, window_height_2);
        }};
        struct LeftPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize             = Resources::getWindowSize();
            const auto window_height_2     = winSize.y - bottom_bar_height_total - padding_y - top_bar_height_total;
            const auto left_window_width_2 = winSize.x - right_window_width - padding_x - (padding_x / 2.0f);
            const auto x                   = (padding_x / 2.0f) + (left_window_width_2 / 2.0f);
            const auto y                   = winSize.y - (padding_y / 2.0f) - (window_height_2 / 2.0f) - (top_bar_height_total);
            return glm::vec2(x, y);
        }};
        m_SetupShipSelectorWindow->setPositionFunctor(LeftPositionFunctor());
        m_SetupShipSelectorWindow->setSizeFunctor(LeftSizeFunctor());
    }
    {
        m_SetupServerInfoWindow = NEW FFAServerInfo(*this, font,
            winSize.x - (right_window_width / 2.0f) - padding_x,
            winSize.y - (padding_y / 2.0f) - (window_height / 2.0f),
            right_window_width,
            window_height,
        0.1f, 1, "Server Information");

        struct RightSizeFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize         = Resources::getWindowSize();
            const auto window_height_2 = winSize.y - bottom_bar_height_total - padding_y - top_bar_height_total;
            return glm::vec2(right_window_width, window_height_2);
        }};
        struct RightPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize         = Resources::getWindowSize();
            const auto window_height_2 = winSize.y - bottom_bar_height_total - padding_y - top_bar_height_total;
            const auto x               = winSize.x - (right_window_width / 2.0f) - (padding_x / 2.0f);
            const auto y               = winSize.y - (padding_y / 2.0f) - (window_height_2 / 2.0f) - (top_bar_height_total);
            return glm::vec2(x, y);
        }};
        m_SetupServerInfoWindow->setPositionFunctor(RightPositionFunctor());
        m_SetupServerInfoWindow->setSizeFunctor(RightSizeFunctor());
    }
}
HostScreenFFA2::~HostScreenFFA2() {
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
    SAFE_DELETE(m_BackgroundEdgeGraphicTop);
    SAFE_DELETE(m_TopLabel);

    SAFE_DELETE(m_SetupShipSelectorWindow);
    SAFE_DELETE(m_SetupServerInfoWindow);
}
void HostScreenFFA2::setTopText(const string& text) {
    m_TopLabel->setText(text);
}
Menu& HostScreenFFA2::getMenu() {
    return m_Menu;
}
const bool HostScreenFFA2::isPersistent() const {
    return m_IsPersistent;
}
void HostScreenFFA2::setPersistent(const bool& persistent) {
    if (persistent) {
        if (Server::PERSISTENT_INFO == false) {
            return;
        }
        m_IsPersistent = true;

        TextBox& your_name = m_SetupServerInfoWindow->getYourNameTextBox();
        TextBox& server_port = m_SetupServerInfoWindow->getServerPortTextBox();

        your_name.setText(Server::PERSISTENT_INFO.getOwnerName());
        server_port.setText(Server::PERSISTENT_INFO.getServerName());

        your_name.lock();
        server_port.lock();
    }else{
        m_IsPersistent = false;

        TextBox& your_name = m_SetupServerInfoWindow->getYourNameTextBox();
        TextBox& server_port = m_SetupServerInfoWindow->getServerPortTextBox();

        your_name.setText(ConfigFile::CONFIG_DATA.host_server_player_name);
        server_port.setText(to_string(ConfigFile::CONFIG_DATA.host_server_port));

        your_name.unlock();
        server_port.unlock();
    }
}

const bool HostScreenFFA2::validateShipSelector() {
    if (m_SetupShipSelectorWindow->getAllowedShips().size() <= 0) {
        m_Menu.setErrorText("Please add at least one allowed ship");
        return false;
    }
    return true;
}
const bool HostScreenFFA2::validateMaxNumPlayersTextBox() {
    auto max_players_text = m_SetupServerInfoWindow->getMaxPlayersTextBox().text();

    if (max_players_text.empty()) {
        m_Menu.setErrorText("Max players cannot be empty");
        return false;
    }

    if (max_players_text.find_first_not_of("0123456789") != std::string::npos) { //numbers only please
        m_Menu.setErrorText("Max players needs to be a number");
        return false;
    }

    try {
        int num = stoi(max_players_text);
    }catch (const std::invalid_argument) { //If no conversion could be performed, an invalid_argument exception is thrown.
        m_Menu.setErrorText("Max players needs to be a number");
        return false;
    }catch (const std::out_of_range) { //If the value read is out of the range of representable values by an int, an out_of_range exception is thrown.
        m_Menu.setErrorText("Max players is out of range of possible values");
        return false;
    }catch (...) {
        m_Menu.setErrorText("Max players is invalid");
        return false;
    }
    return true;
}
const bool HostScreenFFA2::validateServerPortTextBox() {
    auto port_text = m_SetupServerInfoWindow->getServerPortTextBox().text();

    if (port_text.empty()) {
        m_Menu.setErrorText("The server port cannot be empty");
        return false;
    }

    if (port_text.find_first_not_of("0123456789") != std::string::npos) { //numbers only please
        m_Menu.setErrorText("Server port needs to be a number");
        return false;
    }

    try {
        int num = stoi(port_text);
    }catch (const std::invalid_argument) { //If no conversion could be performed, an invalid_argument exception is thrown.
        m_Menu.setErrorText("Server port needs to be a number");
        return false;
    }catch (const std::out_of_range) { //If the value read is out of the range of representable values by an int, an out_of_range exception is thrown.
        m_Menu.setErrorText("Server port is out of range of possible values");
        return false;
    }catch (...) {
        m_Menu.setErrorText("Server port is invalid");
        return false;
    }
    return true;
}
const bool HostScreenFFA2::validateUsernameTextBox() {
    auto username = m_SetupServerInfoWindow->getYourNameTextBox().text();
    const bool space_check = username.find_first_of(' ') != std::string::npos;
    const bool letters_check = !(std::regex_match(username, std::regex("[a-zA-ZäöüßÄÖÜ]+")));
    if (username.empty()) {
        m_Menu.setErrorText("The username cannot be empty");
        return false;
    }
    if (space_check) { //no spaces please
        m_Menu.setErrorText("The username cannot contain spaces");
        return false;
    }
    if (letters_check) { //letters only please
        m_Menu.setErrorText("The username must only contain letters");
        return false;
    }
    return true;
}
const bool HostScreenFFA2::validateMatchDurationTextBox() {
    auto text_box_text = m_SetupServerInfoWindow->getMatchDurationTextBox().text();
    auto list = Helper::SeparateStringByCharacter(text_box_text, ':');
    if (list.size() == 2) {
        for (auto& number_as_str : list) {
            if (number_as_str.find_first_not_of("0123456789") != std::string::npos) { //numbers only please
                m_Menu.setErrorText("Match duration must only contain numbers");
                return false;
            }
            try {
                int num = stoi(number_as_str);
            }catch (const std::invalid_argument) { //If no conversion could be performed, an invalid_argument exception is thrown.
                m_Menu.setErrorText("Match duration needs to be in minutes : seconds format");
                return false;
            }catch (const std::out_of_range) { //If the value read is out of the range of representable values by an int, an out_of_range exception is thrown.
                m_Menu.setErrorText("Match duration is out of range of possible values");
                return false;
            }catch (...) {
                m_Menu.setErrorText("Match duration is invalid");
                return false;
            }
        }
        return true;
    }
    m_Menu.setErrorText("Match duration is invalid");
    return false;
}
const bool HostScreenFFA2::validateLobbyDurationTextBox() {
    auto text_box_text = m_SetupServerInfoWindow->getLobbyDurationTextBox().text();
    auto list = Helper::SeparateStringByCharacter(text_box_text, ':');
    if (list.size() == 2) {
        for (auto& number_as_str : list) {
            if (number_as_str.find_first_not_of("0123456789") != std::string::npos) { //numbers only please
                m_Menu.setErrorText("Lobby duration must only contain numbers");
                return false;
            }
            try {
                int num = stoi(number_as_str);
            }catch (const std::invalid_argument) { //If no conversion could be performed, an invalid_argument exception is thrown.
                m_Menu.setErrorText("Lobby duration needs to be in minutes : seconds format");
                return false;
            }catch (const std::out_of_range) { //If the value read is out of the range of representable values by an int, an out_of_range exception is thrown.
                m_Menu.setErrorText("Lobby duration is out of range of possible values");
                return false;
            }catch (...) {
                m_Menu.setErrorText("Lobby duration is invalid");
                return false;
            }
        }
        return true;
    }
    m_Menu.setErrorText("Lobby duration is invalid");
    return false;
}
const unsigned int HostScreenFFA2::get_duration_sec_helper(TextBox& box) {
    auto text_box_text = box.text();
    auto list = Helper::SeparateStringByCharacter(text_box_text, ':');
    return (stoi(list[0]) * 60) + stoi(list[1]);
}
const float HostScreenFFA2::get_duration_min_helper(TextBox& box) {
    auto text_box_text = box.text();
    auto list = Helper::SeparateStringByCharacter(text_box_text, ':');
    float mins = static_cast<float>(stoi(list[0]));
    float secs = static_cast<float>(stoi(list[1]));
    return mins + (secs / 60.0f);
}
const unsigned int HostScreenFFA2::getMatchDurationFromTextBoxInSeconds() {
    return get_duration_sec_helper(m_SetupServerInfoWindow->getMatchDurationTextBox());
}
const float HostScreenFFA2::getMatchDurationFromTextBoxInMinutes() {
    return get_duration_min_helper(m_SetupServerInfoWindow->getMatchDurationTextBox());
}
const unsigned int HostScreenFFA2::getLobbyDurationFromTextBoxInSeconds() {
    return get_duration_sec_helper(m_SetupServerInfoWindow->getLobbyDurationTextBox());
}
const float HostScreenFFA2::getLobbyDurationFromTextBoxInMinutes() {
    return get_duration_min_helper(m_SetupServerInfoWindow->getLobbyDurationTextBox());
}
void HostScreenFFA2::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::vec2(newWidth, newHeight);

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), bottom_bar_height_total / 2.0f);

    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, 0);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);

    m_BackgroundEdgeGraphicTop->setSize(winSize.x, top_bar_height_total);
    m_BackgroundEdgeGraphicTop->setPosition(winSize.x / 2.0f, winSize.y);

    m_TopLabel->setPosition(winSize.x / 2.0f, winSize.y - (top_bar_height_total / 2.0f) + 15.0f);

    m_SetupShipSelectorWindow->onResize(newWidth, newHeight);
    m_SetupServerInfoWindow->onResize(newWidth, newHeight);
}

void HostScreenFFA2::update(const double& dt) {
    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackgroundEdgeGraphicTop->update(dt);

    m_TopLabel->update(dt);

    m_SetupShipSelectorWindow->update(dt);
    m_SetupServerInfoWindow->update(dt);
}
void HostScreenFFA2::render() {
    m_BackButton->render();
    m_ForwardButton->render();

    m_BackgroundEdgeGraphicBottom->render();
    m_BackgroundEdgeGraphicTop->render();

    m_TopLabel->render();

    m_SetupShipSelectorWindow->render();
    m_SetupServerInfoWindow->render();
}
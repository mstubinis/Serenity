#include "HostScreenTeamDeathmatch2.h"
#include "HostScreen1.h"
#include "MapDescriptionWindow.h"
#include "ServerLobbyChatWindow.h"
#include "../../Menu.h"
#include "../../factions/Faction.h"
#include "../../Core.h"
#include "../Button.h"
#include "../TextBox.h"
#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../networking/server/Server.h"

#include "../../teams/TeamIncludes.h"
#include "../../teams/Team.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <regex>

using namespace Engine;
using namespace std;

constexpr auto padding_x = 100.0f;
constexpr auto padding_y = 100.0f;
constexpr auto bottom_bar_height = 50.0f;
constexpr auto bottom_bar_button_width = 150.0f;

constexpr auto bottom_bar_height_total = 80;
constexpr auto top_bar_height_total = 60;

struct Host2TD_ButtonBack_OnClick { void operator()(Button* button) const {
    auto& hostScreenTD = *static_cast<HostScreenTeamDeathmatch2*>(button->getUserPointer());

    auto& menu = hostScreenTD.getMenu();
    menu.setGameState(GameState::Host_Screen_Setup_1);

    //force server to disconnect client
    menu.getCore().shutdownClient();
    menu.getCore().shutdownServer();
}};
struct Host2TD_ButtonNext_OnClick { void operator()(Button* button) const {
    auto& hostScreenTD       = *static_cast<HostScreenTeamDeathmatch2*>(button->getUserPointer());
    auto& menu               = hostScreenTD.getMenu();
    const string& username   = hostScreenTD.m_UserName_TextBox->text();
    const string& portstring = hostScreenTD.m_ServerPort_TextBox->text();
    const auto& map          = Server::SERVER_HOST_DATA.getMapChoice();
    if (!portstring.empty() && !username.empty() && !map.map_file_path.empty()) {
        //TODO: prevent special characters in usename
        if (std::regex_match(portstring, std::regex("^(0|[1-9][0-9]*)$"))) { //port must have numbers only
            if (username.find_first_not_of(' ') != std::string::npos) {
                if (std::regex_match(username, std::regex("[a-zA-ZäöüßÄÖÜ]+"))) { //letters only please

                    auto& core = menu.getCore();

                    const int port = stoi(portstring);
                    core.startServer(port);
                    core.startClient(nullptr, port, username, "127.0.0.1"); //the client will request validation at this stage


                    //TODO: replace this hard coded test case with real input values
                    vector<TeamNumber::Enum> nil;
                    vector<TeamNumber::Enum> team1enemies{ TeamNumber::Team_2 };
                    vector<TeamNumber::Enum> team2enemies{ TeamNumber::Team_1 };
                    Team team1 = Team(TeamNumber::Team_1, nil, team1enemies);
                    Team team2 = Team(TeamNumber::Team_2, nil, team2enemies);
                    Server::SERVER_HOST_DATA.setGameplayModeType(GameplayModeType::TeamDeathmatch);
                    Server::SERVER_HOST_DATA.setMaxAmountOfPlayers(50);
                    Server::SERVER_HOST_DATA.addTeam(team1);
                    Server::SERVER_HOST_DATA.addTeam(team2);
                    core.getClient()->getGameplayMode() = Server::SERVER_HOST_DATA.getGameplayMode();

                    core.getServer()->startupMap(map);

                    menu.m_ServerLobbyChatWindow->setUserPointer(core.getClient());


                    menu.setGameState(GameState::Host_Screen_Lobby_FFA_3);
                    menu.setErrorText("");

                }else {
                    menu.setErrorText("The username must only contain letters");
                }
            }else {
                menu.setErrorText("The username must have some letters in it");
            }
        }else {
            menu.setErrorText("Server port must contain numbers only");
        }
    }else {
        menu.setErrorText("Do not leave any fields blank");
    }
}};


HostScreenTeamDeathmatch2::HostScreenTeamDeathmatch2(HostScreen1& hostScreen1, Menu& menu, Font& font) : m_HostScreen1(hostScreen1), m_Menu(menu) {
    const auto winSize = glm::vec2(Resources::getWindowSize());
    const auto contentSize = winSize - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;

    m_BackgroundEdgeGraphicTop = NEW Button(font, winSize.x / 2.0f, winSize.y, winSize.x, top_bar_height_total);
    m_BackgroundEdgeGraphicTop->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicTop->setAlignment(Alignment::TopCenter);
    m_BackgroundEdgeGraphicTop->setDepth(0.2f);
    m_BackgroundEdgeGraphicTop->disable();
    m_BackgroundEdgeGraphicTop->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicTop->enableTextureCorner(false);
    m_TopLabel = new Text(winSize.x / 2.0f, winSize.y - (top_bar_height_total / 2.0f) + 15.0f, font);
    m_TopLabel->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_TopLabel->setAlignment(Alignment::Center);
    m_TopLabel->setTextAlignment(TextAlignment::Center);

    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, bottom_bar_height_total / 2.0f, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicBottom->setDepth(0.2f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host2TD_ButtonBack_OnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(Host2TD_ButtonNext_OnClick());
}
HostScreenTeamDeathmatch2::~HostScreenTeamDeathmatch2() {
    SAFE_DELETE(m_UserName_TextBox);
    SAFE_DELETE(m_ServerPort_TextBox);
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
    SAFE_DELETE(m_BackgroundEdgeGraphicTop);
    SAFE_DELETE(m_TopLabel);
}
void HostScreenTeamDeathmatch2::setTopText(const string& text) {
    m_TopLabel->setText(text);
}
Menu& HostScreenTeamDeathmatch2::getMenu() {
    return m_Menu;
}
void HostScreenTeamDeathmatch2::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::vec2(glm::uvec2(newWidth, newHeight));

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);

    m_BackgroundEdgeGraphicTop->setSize(winSize.x, top_bar_height_total);
    m_BackgroundEdgeGraphicTop->setPosition(winSize.x / 2.0f, winSize.y);

    m_TopLabel->setPosition(winSize.x / 2.0f, winSize.y - (top_bar_height_total / 2.0f) + 15.0f);
}

void HostScreenTeamDeathmatch2::update(const double& dt) {
    //m_ServerPort_TextBox->update(dt);
    //m_UserName_TextBox->update(dt);

    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackgroundEdgeGraphicTop->update(dt);

    m_TopLabel->update(dt);
}
void HostScreenTeamDeathmatch2::render() {
    //m_ServerPort_TextBox->render();
    //m_UserName_TextBox->render();

    m_BackButton->render();
    m_ForwardButton->render();

    m_BackgroundEdgeGraphicBottom->render();
    m_BackgroundEdgeGraphicTop->render();

    m_TopLabel->render();
}
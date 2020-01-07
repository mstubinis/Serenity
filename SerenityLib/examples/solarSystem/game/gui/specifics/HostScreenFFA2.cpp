#include "HostScreenFFA2.h"
#include "HostScreen.h"
#include "MapDescriptionWindow.h"
#include "ServerLobbyChatWindow.h"
#include "../../Menu.h"
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

const auto padding_x               = 100.0f;
const auto padding_y               = 100.0f;
const auto bottom_bar_height       = 50.0f;
const auto bottom_bar_button_width = 150.0f;

const auto bottom_bar_height_total = 80;

struct Host2FFA_ButtonBack_OnClick { void operator()(Button* button) const {
    auto& hostScreenFFA = *static_cast<HostScreenFFA2*>(button->getUserPointer());

    auto& menu = hostScreenFFA.getMenu();
    menu.setGameState(GameState::Host_Screen_Setup_1);

    //force server to disconnect client
    menu.getCore().shutdownClient();
    menu.getCore().shutdownServer();
}};
struct Host2FFA_ButtonNext_OnClick { void operator()(Button* button) const {
    auto& hostScreenFFA = *static_cast<HostScreenFFA2*>(button->getUserPointer());

    auto& menu = hostScreenFFA.getMenu();

    const string& username   = hostScreenFFA.m_UserName_TextBox->text();
    const string& portstring = hostScreenFFA.m_ServerPort_TextBox->text();
    const auto& map          = hostScreenFFA.m_HostScreen1.getCurrentChoice();
    if (!portstring.empty() && !username.empty() && !map.map_file_path.empty()) {

        auto& core = menu.getCore();
        menu.setGameState(GameState::Host_Screen_Lobby_3);
        menu.setErrorText("");
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
                    core.getServer()->getGameplayMode().setGameplayMode(GameplayModeType::TeamDeathmatch);
                    core.getServer()->getGameplayMode().setMaxAmountOfPlayers(50);
                    core.getServer()->getGameplayMode().addTeam(team1);
                    core.getServer()->getGameplayMode().addTeam(team2);
                    core.getClient()->getGameplayMode() = core.getServer()->getGameplayMode();

                    core.getServer()->startupMap(map);

                    menu.m_ServerLobbyChatWindow->setUserPointer(core.getClient());
        
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


HostScreenFFA2::HostScreenFFA2(HostScreen& hostScreen1, Menu& menu, Font& font) : m_Menu(menu), m_HostScreen1(hostScreen1) {
    const auto winSize = Resources::getWindowSize();
    const auto contentSize = glm::vec2(winSize) - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;


    m_BackgroundEdgeGraphicTop = NEW Button(font, winSize.x / 2.0f, winSize.y, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicTop->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlueDark]);
    m_BackgroundEdgeGraphicTop->setAlignment(Alignment::TopCenter);
    m_BackgroundEdgeGraphicTop->setDepth(0.512f);
    m_BackgroundEdgeGraphicTop->disable();
    m_BackgroundEdgeGraphicTop->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicTop->enableTextureCorner(false);
    m_TopLabel = new Text(winSize.x / 2.0f, winSize.y - (bottom_bar_height_total / 2.0f) + 15.0f, font);
    m_TopLabel->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_TopLabel->setAlignment(Alignment::Center);
    m_TopLabel->setTextAlignment(TextAlignment::Center);

    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, 0, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlueDark]);
    m_BackgroundEdgeGraphicBottom->setAlignment(Alignment::BottomCenter);
    m_BackgroundEdgeGraphicBottom->setDepth(0.512f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host2FFA_ButtonBack_OnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(Host2FFA_ButtonNext_OnClick());

    m_UserName_TextBox = NEW TextBox("Your Name", font, 20, winSize.x / 2.0f, 275.0f);
    m_UserName_TextBox->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_UserName_TextBox->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);

    m_ServerPort_TextBox = NEW TextBox("Server Port", font, 7, winSize.x / 2.0f, 195.0f);
    m_ServerPort_TextBox->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ServerPort_TextBox->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ServerPort_TextBox->setText("55000");
}
HostScreenFFA2::~HostScreenFFA2() {
    SAFE_DELETE(m_UserName_TextBox);
    SAFE_DELETE(m_ServerPort_TextBox);
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
    SAFE_DELETE(m_BackgroundEdgeGraphicTop);
    SAFE_DELETE(m_TopLabel);
}
void HostScreenFFA2::setTopText(const string& text) {
    m_TopLabel->setText(text);
}
Menu& HostScreenFFA2::getMenu() {
    return m_Menu;
}
void HostScreenFFA2::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::uvec2(newWidth, newHeight);

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), bottom_bar_height_total / 2.0f);

    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, 0);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);

    m_BackgroundEdgeGraphicTop->setSize(winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicTop->setPosition(winSize.x / 2.0f, winSize.y);

    m_TopLabel->setPosition(winSize.x / 2.0f, winSize.y - (bottom_bar_height_total / 2.0f) + 15.0f);
}

void HostScreenFFA2::update(const double& dt) {
    m_ServerPort_TextBox->update(dt);
    m_UserName_TextBox->update(dt);

    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackgroundEdgeGraphicTop->update(dt);

    m_TopLabel->update(dt);
}
void HostScreenFFA2::render() {
    m_ServerPort_TextBox->render();
    m_UserName_TextBox->render();

    m_BackButton->render();
    m_ForwardButton->render();

    m_BackgroundEdgeGraphicBottom->render();
    m_BackgroundEdgeGraphicTop->render();

    m_TopLabel->render();
}
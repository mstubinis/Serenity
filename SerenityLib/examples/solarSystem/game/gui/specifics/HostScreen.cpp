#include "HostScreen.h"
#include "ServerHostingMapSelectorWindow.h"
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

#include <regex>

using namespace Engine;
using namespace std;

const auto padding_x = 40.0f;
const auto padding_y = 40.0f;
const auto bottom_bar_height = 50.0f;
const auto bottom_bar_button_width = 150.0f;

struct Host_ButtonBack_OnClick { void operator()(Button* button) const {
    HostScreen& hostScreen = *static_cast<HostScreen*>(button->getUserPointer());
    auto& menu = hostScreen.getMenu();
    menu.go_to_main_menu();

    //force server to disconnect client
    menu.getCore().shutdownClient();
    menu.getCore().shutdownServer();
}};
struct Host_ButtonNext_OnClick { void operator()(Button* button) const {
    HostScreen& hostScreen = *static_cast<HostScreen*>(button->getUserPointer());
    auto& menu = hostScreen.getMenu();

    const string& username   = hostScreen.getUserNameTextBox().text();
    const string& portstring = hostScreen.getServerPortTextBox().text();
    const auto& map          = hostScreen.getMapSelectionWindow().getCurrentChoice();
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


HostScreen::HostScreen(Menu& menu, Font& font) : m_Menu(menu){

    const auto winSize = Resources::getWindowSize();
    const auto contentSize = glm::vec2(winSize) - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);

    m_BackgroundEdgeGraphic = NEW Button(font, winSize.x/2.0f, winSize.y/2.0f, winSize.x - padding_x, winSize.y - padding_y);
    m_BackgroundEdgeGraphic->setTexture(nullptr);
    m_BackgroundEdgeGraphic->setColor(0.5f, 0.78f, 0.94f, 1.0f);
    m_BackgroundEdgeGraphic->setDepth(0.512f);
    m_BackgroundEdgeGraphic->disable();
    m_BackgroundEdgeGraphic->disableCenterTexture();

    m_ServerHostMapSelector = NEW ServerHostingMapSelectorWindow(*this, font, padding_x, winSize.y - padding_y, contentSize.x / 3.0f, contentSize.y / 2.0f);
    m_MapDescriptionWindow = NEW MapDescriptionWindow(font, padding_x + contentSize.x / 3.0f, winSize.y - padding_y, contentSize.x / 3.0f, contentSize.y / 2.0f);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_BackButton->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_BackButton->setTextureCorner(nullptr);
    m_BackButton->setTextureEdge(nullptr);

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ForwardButton->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ForwardButton->setTextureCorner(nullptr);
    m_ForwardButton->setTextureEdge(nullptr);

    m_BackButton->setUserPointer(this);
    m_ForwardButton->setUserPointer(this);

    m_BackButton->setOnClickFunctor(Host_ButtonBack_OnClick());
    m_ForwardButton->setOnClickFunctor(Host_ButtonNext_OnClick());


    m_UserName_TextBox = NEW TextBox("Your Name", font, 20, winSize.x / 2.0f, 275.0f);
    m_UserName_TextBox->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_UserName_TextBox->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);

    m_ServerPort_TextBox = NEW TextBox("Server Port", font, 7, winSize.x / 2.0f, 195.0f);
    m_ServerPort_TextBox->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ServerPort_TextBox->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ServerPort_TextBox->setText("55000");

    //m_Info_Text = NEW Text(Resources::getWindowSize().x / 2.0f, 65.0f, font);
    //m_Info_Text->setTextAlignment(TextAlignment::Center);

}
HostScreen::~HostScreen() {
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_UserName_TextBox);
    SAFE_DELETE(m_ServerPort_TextBox);
    //SAFE_DELETE(m_Info_Text);
    SAFE_DELETE(m_MapDescriptionWindow);
    SAFE_DELETE(m_ServerHostMapSelector);
    SAFE_DELETE(m_BackgroundEdgeGraphic);
}
Menu& HostScreen::getMenu() {
    return m_Menu;
}
TextBox& HostScreen::getUserNameTextBox() {
    return *m_UserName_TextBox;
}
TextBox& HostScreen::getServerPortTextBox() {
    return *m_ServerPort_TextBox;
}
ServerHostingMapSelectorWindow& HostScreen::getMapSelectionWindow() {
    return *m_ServerHostMapSelector;
}
MapDescriptionWindow& HostScreen::getMapDescriptionWindow() {
    return *m_MapDescriptionWindow;
}
void HostScreen::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::uvec2(newWidth, newHeight);

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f));
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f));

    m_ServerHostMapSelector->setPosition(padding_x - 4, winSize.y - (padding_y - 2));
    const auto contentSize = winSize - glm::uvec2(padding_x * 2.0f, (padding_y * 2.0f) );
    const auto map_selector_width = static_cast<float>(contentSize.x) / 3.0f;
    const auto map_selector_height = static_cast<float>(contentSize.y) / 2.0f;
    m_ServerHostMapSelector->setSize(map_selector_width, map_selector_height);

    m_BackgroundEdgeGraphic->setPosition(winSize.x / 2.0f, winSize.y / 2.0f);
    m_BackgroundEdgeGraphic->setSize(winSize.x - padding_x, winSize.y - padding_y);

    m_MapDescriptionWindow->setPosition(m_ServerHostMapSelector->getWindowFrame().positionWorld().x + m_ServerHostMapSelector->getWindowFrame().width() + 2, winSize.y - (padding_y - 2));
    m_MapDescriptionWindow->setSize(map_selector_width, map_selector_height + 2);
}

void HostScreen::update(const double& dt) {
    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_ServerPort_TextBox->update(dt);
    m_UserName_TextBox->update(dt);
    m_ServerHostMapSelector->update(dt);
    m_MapDescriptionWindow->update(dt);
    m_BackgroundEdgeGraphic->update(dt);
}
void HostScreen::render() {
    m_ServerHostMapSelector->render();
    m_MapDescriptionWindow->render();
    m_BackButton->render();
    m_ForwardButton->render();

    m_ServerPort_TextBox->render();
    m_UserName_TextBox->render();
    m_BackgroundEdgeGraphic->render();
}
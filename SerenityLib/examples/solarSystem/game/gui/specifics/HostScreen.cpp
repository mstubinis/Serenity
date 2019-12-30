#include "HostScreen.h"
#include "../RoundedWindow.h"
#include "MapSelectionWindow.h"
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

const auto left_window_width       = 550;

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
    const auto winSize                 = Resources::getWindowSize();
    const auto contentSize             = glm::vec2(winSize) - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height      = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;

    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, bottom_bar_height_total / 2.0f, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlueDark]);
    m_BackgroundEdgeGraphicBottom->setDepth(0.512f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
    {
        m_LeftWindow = new MapSelectionWindow(*this,font, (padding_x / 2.0f) + (left_window_width / 2.0f), winSize.y - (padding_y / 2.0f) - (window_height / 2.0f), left_window_width, window_height, 0.512f, 1, "Free for All");

        struct LeftSizeFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2(left_window_width, window_height);
        }};
        struct LeftPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2((padding_x / 2.0f) + (left_window_width / 2.0f), winSize.y - (padding_y / 2.0f) - (window_height / 2.0f));
        }};
        m_LeftWindow->setPositionFunctor(LeftPositionFunctor());
        m_LeftWindow->setSizeFunctor(LeftSizeFunctor());

    }
    {
        m_RightWindow = new RoundedWindow(font, winSize.x - (padding_x / 2.0f) - (window_height / 2.0f), winSize.y - (padding_y / 2.0f) - (window_height / 2.0f), window_height, window_height,0.512f, 1, "");
        struct RightSizeFunctor {glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2(window_height, window_height);
        }};
        struct RightPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2(winSize.x - (padding_x / 2.0f) - (window_height / 2.0f), winSize.y - (padding_y / 2.0f) - (window_height / 2.0f));
        }};
        m_RightWindow->setPositionFunctor(RightPositionFunctor());
        m_RightWindow->setSizeFunctor(RightSizeFunctor());
    }
    m_MapDescriptionWindow = NEW MapDescriptionWindow(font, padding_x + contentSize.x / 3.0f, winSize.y - padding_y, contentSize.x / 3.0f, contentSize.y / 2.0f);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host_ButtonBack_OnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(Host_ButtonNext_OnClick());


    m_UserName_TextBox = NEW TextBox("Your Name", font, 20, winSize.x / 2.0f, 275.0f);
    m_UserName_TextBox->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_UserName_TextBox->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);

    m_ServerPort_TextBox = NEW TextBox("Server Port", font, 7, winSize.x / 2.0f, 195.0f);
    m_ServerPort_TextBox->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ServerPort_TextBox->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ServerPort_TextBox->setText("55000");
}
HostScreen::~HostScreen() {
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_UserName_TextBox);
    SAFE_DELETE(m_ServerPort_TextBox);
    SAFE_DELETE(m_MapDescriptionWindow);
    {
        SAFE_DELETE(m_LeftWindow);
    }
    {
        SAFE_DELETE(m_RightWindow);
    }

    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
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
MapSelectionWindow& HostScreen::getMapSelectionWindow() {
    return *m_LeftWindow;
}
MapDescriptionWindow& HostScreen::getMapDescriptionWindow() {
    return *m_MapDescriptionWindow;
}
void HostScreen::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::uvec2(newWidth, newHeight);
    const auto contentSize = glm::vec2(winSize) - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;
    const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);

    m_BackButton->setPosition(
        padding_x + (bottom_bar_button_width / 2.0f),
        bottom_bar_height_total / 2.0f
    );
    m_ForwardButton->setPosition(
        winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), 
        bottom_bar_height_total / 2.0f
    );


    {
        m_LeftWindow->onResize(newWidth, newHeight);
    }
    {
        m_RightWindow->onResize(newWidth, newHeight);

    }

    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);

    //m_MapDescriptionWindow->setPosition(m_LeftWindow->getWindowFrame().positionWorld().x + m_LeftWindow->getWindowFrame().width() + 2, winSize.y - (padding_y - 2));
    m_MapDescriptionWindow->setSize(first_2_boxes_width_top / 2.0f, top_content_height + 2);
}

void HostScreen::update(const double& dt) {
    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_ServerPort_TextBox->update(dt);
    m_UserName_TextBox->update(dt);
    m_MapDescriptionWindow->update(dt);
    {
        m_LeftWindow->update(dt);
    }
    {
        m_RightWindow->update(dt);
    }

    m_BackgroundEdgeGraphicBottom->update(dt);
}
void HostScreen::render() {
    m_MapDescriptionWindow->render();
    m_BackButton->render();
    m_ForwardButton->render();

    m_ServerPort_TextBox->render();
    m_UserName_TextBox->render();
    {
        m_LeftWindow->render();
    }
    {
        m_RightWindow->render();
    }

    m_BackgroundEdgeGraphicBottom->render();
    //render map screenshot
    const auto& current_map_data = getMapSelectionWindow().getCurrentChoice();

    auto& desc_frame = getMapDescriptionWindow().getWindowFrame();

    const auto texture_frame_size = glm::vec2(desc_frame.height(), desc_frame.height());
    const auto ss_pos = desc_frame.positionWorld() + glm::vec2(desc_frame.width(),0);
    if (!current_map_data.map_name.empty()) {
        Texture& texture = *(Texture*)current_map_data.map_screenshot_handle.get();
        auto scl = texture_frame_size / glm::vec2(texture.size());
        Renderer::renderTexture(texture, ss_pos + glm::vec2(4, 0), glm::vec4(1.0f), 0.0f, scl, 0.0141f, Alignment::TopLeft);
    }
    Renderer::renderBorder(1, ss_pos + glm::vec2(3, 0), glm::vec4(1, 1, 0, 1), texture_frame_size.x + 1, texture_frame_size.y, 0.0f, 0.014f, Alignment::TopLeft);
}
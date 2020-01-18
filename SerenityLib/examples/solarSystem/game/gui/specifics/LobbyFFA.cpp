#include "LobbyFFA.h"
#include "FFALobbyShipSelector.h"
#include "MessageWithArrow.h"
#include "Ship3DViewer.h"
#include "ShipDescription.h"

#include "../../gui/Button.h"
#include "../../gui/ScrollFrame.h"

#include "../../factions/Faction.h"
#include "../../Menu.h"
#include "../../Core.h"
#include "../../Helper.h"
#include "../../networking/server/Server.h"
#include "../../networking/client/Client.h"
#include "../../networking/client/ClientMapSpecificData.h"
#include "../../networking/packets/PacketConnectionAccepted.h"
#include "../../networking/packets/PacketMessage.h"
#include "../../networking/packets/PacketUpdateLobbyTimeLeft.h"
#include "../../map/Map.h"
#include "../../teams/Team.h"
#include "../../gui/RoundedWindow.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace std;
using namespace Engine;

constexpr auto padding_x               = 100.0f;
constexpr auto padding_y               = 100.0f;
constexpr auto bottom_bar_height       = 50.0f;
constexpr auto bottom_bar_button_width = 150.0f;

constexpr auto bottom_bar_height_total = 80.0f;
constexpr auto top_bar_height_total    = 60.0f;

constexpr auto inner_padding_x         = 30.0f;
constexpr auto inner_padding_y         = 55.0f;

constexpr auto chat_box_height         = 340.0f;

struct FFALobbyBackOnClick final { void operator()(Button* button) const {
    auto& lobby = *static_cast<LobbyScreenFFA*>(button->getUserPointer());
    if (lobby.isHost()) {
        lobby.m_Menu.setGameState(GameState::Host_Screen_Setup_FFA_2);

        //force server to disconnect client
        lobby.m_Core.shutdownClient();
        lobby.m_Core.shutdownServer();
    }else{
        //TODO: go to the join game screen

        lobby.m_Core.shutdownClient();
    }
    lobby.setChosenShipClass("");
}};
struct FFALobbyForwardOnClick final { void operator()(Button* button) const {
    auto& lobby = *static_cast<LobbyScreenFFA*>(button->getUserPointer());
    if (lobby.isHost()) { //redundant (as only the host should see this button...), but it's safe...
        if (lobby.m_TimeLeftUntilMatchStarts > 6.0) {
            PacketUpdateLobbyTimeLeft pOut;
            pOut.PacketType = PacketType::Server_To_Client_Update_Lobby_Time_Left;
            pOut.time_left = 5.5;
            lobby.m_Core.getServer()->send_to_all(pOut);
        }
    }
}};

LobbyScreenFFA::LobbyScreenFFA(Font& font, Menu& menu, Core& core, Scene& scene, Camera& camera): m_Menu(menu), m_Core(core), m_Font(font){
    const auto& winSize = glm::vec2(Resources::getWindowSize());
   
    m_TimeLeftUntilMatchStarts = 0.0;

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
    m_BackButton->setOnClickFunctor(FFALobbyBackOnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Start");
    m_ForwardButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(FFALobbyForwardOnClick());
    
    m_RoundedWindow = NEW RoundedWindow(font, 
        (winSize.x / 2.0f),
        (winSize.y / 2.0f) + (bottom_bar_height_total / 2.0f),
        winSize.x - padding_x,
        winSize.y - padding_y - bottom_bar_height_total, 
    0.2f, 1, "");
    struct SizeFunctor { glm::vec2 operator()(RoundedWindow* window) const {
        const auto winSize = glm::vec2(Resources::getWindowSize());
        return glm::vec2(winSize.x - padding_x, winSize.y - padding_y - bottom_bar_height_total);
    }};
    struct PositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
        const auto winSize = glm::vec2(Resources::getWindowSize());
        return glm::vec2((winSize.x / 2.0f), (winSize.y / 2.0f) + (bottom_bar_height_total / 2.0f));
    }};
    m_RoundedWindow->setPositionFunctor(PositionFunctor());
    m_RoundedWindow->setSizeFunctor(SizeFunctor());


    const auto rounded_win_size = glm::vec2(m_RoundedWindow->width(), m_RoundedWindow->height());
    const auto ends_size = rounded_win_size.y - chat_box_height;

    m_ShipSelector = NEW FFALobbyShipSelector(*this, font, 
        (padding_x / 2.0f) + inner_padding_x,
        winSize.y - (padding_y / 2.0f) - inner_padding_y,
        ends_size * 1.2f,
        ends_size,
    0.17f, 1, "Ship Database");

    m_Ship3DViewer = new Ship3DViewer(core, scene, camera, 
        winSize.x - ((padding_x / 2.0f) + inner_padding_x),
        winSize.y - (padding_y / 2.0f) - inner_padding_y,
        ends_size * 1.2f,
        ends_size
    );
    m_Ship3DViewer->hide();


    m_ShipDescription = new ShipDescription(font, 
        (padding_x / 2.0f) + inner_padding_x + (ends_size * 1.2f),
        winSize.y - (padding_y / 2.0f) - inner_padding_y,
        (winSize.x - (padding_x + (inner_padding_x * 2.0f))) - ((ends_size * 1.2f) * 2.0f) - 10.0f,
        ends_size,
    0.15f);

    m_ChooseShipMessage = new MessageWithArrow(font, "Choose a ship. If you do not choose one, you will\nbe given a ship at random when the match starts",
        (padding_x / 2.0f) + inner_padding_x + (ends_size),
        (winSize.y - (padding_y / 2.0f) - inner_padding_y) - 25.0f,
    0.7f);

    setHost(false);
}
LobbyScreenFFA::~LobbyScreenFFA() {
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_BackButton);

    SAFE_DELETE(m_RoundedWindow);
    SAFE_DELETE(m_ShipSelector);
    SAFE_DELETE(m_Ship3DViewer);
    SAFE_DELETE(m_ShipDescription);
    SAFE_DELETE(m_ChooseShipMessage);
}
void LobbyScreenFFA::setTimeLeftUntilMatchStartsInSeconds(const double& seconds) {
    m_TimeLeftUntilMatchStarts = (seconds);
}
void LobbyScreenFFA::showShipViewer() {
    m_Ship3DViewer->show();
}
void LobbyScreenFFA::hideShipViewer() {
    m_Ship3DViewer->hide();
}
void LobbyScreenFFA::setTopLabelText(const string& text) {
    m_RoundedWindow->setLabelText(text);
}
const bool LobbyScreenFFA::isHost() const {
    return m_IsHost;
}
Ship3DViewer& LobbyScreenFFA::getShip3DViewer() {
    return *m_Ship3DViewer;
}
ShipDescription& LobbyScreenFFA::getShipDescriptionWindow() {
    return *m_ShipDescription;
}
FFALobbyShipSelector& LobbyScreenFFA::getShipSelector() {
    return *m_ShipSelector;
}
MessageWithArrow& LobbyScreenFFA::getChooseShipMessageDisplay() {
    return *m_ChooseShipMessage;
}
const bool LobbyScreenFFA::setChosenShipClass(const string& shipClass) {
    const auto& current_ship_class = m_ShipSelector->getChosenShipClass();
    if (current_ship_class == shipClass)
        return false;

    m_ShipSelector->clearChosenShipClass();
    if (shipClass.empty()) {
        m_ShipDescription->setShipClass("");
        m_Ship3DViewer->setShipClass("");
        m_ChooseShipMessage->show();
    }else {
        m_ShipDescription->setShipClass(shipClass);
        m_Ship3DViewer->setShipClass(shipClass);
        m_ChooseShipMessage->hide();
    }
    return true;
}
void LobbyScreenFFA::initShipSelector(const PacketConnectionAccepted& packet) {
    m_ShipSelector->init_window_contents(packet);
}
void LobbyScreenFFA::clearShipSelector() {
    m_ShipSelector->clear();
    m_ChooseShipMessage->show();
}
void LobbyScreenFFA::setHost(const bool host) {
    m_IsHost = host;
    if (host) {
        m_ForwardButton->enable();
        m_ForwardButton->enableMouseover();
        m_ForwardButton->show();
    }else{
        m_ForwardButton->disable();
        m_ForwardButton->disableMouseover();
        m_ForwardButton->hide();
    }
}
void LobbyScreenFFA::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::vec2(glm::uvec2(newWidth, newHeight));

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), (bottom_bar_height_total / 2.0f));
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), (bottom_bar_height_total / 2.0f));

    m_RoundedWindow->onResize(newWidth, newHeight);

    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, 0);

    const auto rounded_win_size = glm::vec2(m_RoundedWindow->width(), m_RoundedWindow->height());
    const auto ends_size = rounded_win_size.y - chat_box_height;
    if (m_ShipSelector) {
        m_ShipSelector->setSize(ends_size * 1.2f, ends_size);
        m_ShipSelector->setPosition( (padding_x / 2.0f) + inner_padding_x, winSize.y - (padding_y / 2.0f) - inner_padding_y );
    }
    if (m_Ship3DViewer) {
        m_Ship3DViewer->setSize(ends_size * 1.2f, ends_size);
        m_Ship3DViewer->setPosition(
            winSize.x - (padding_x / 2.0f) - inner_padding_x - m_Ship3DViewer->getViewportDimensions().z,
            winSize.y - (padding_y / 2.0f) - inner_padding_y - m_Ship3DViewer->getViewportDimensions().w
        );
    }
    if (m_ShipDescription) {
        m_ShipDescription->setSize(  (winSize.x - (padding_x + (inner_padding_x * 2.0f))) - ((ends_size * 1.2f) * 2.0f) - 10.0f, ends_size);
        m_ShipDescription->setPosition(  (padding_x / 2.0f) + inner_padding_x + (ends_size * 1.2f), winSize.y - (padding_y / 2.0f) - inner_padding_y );
    }
    if (m_ChooseShipMessage) {
        m_ChooseShipMessage->setPosition(
            (padding_x / 2.0f) + inner_padding_x + (rounded_win_size.y - chat_box_height),
            (winSize.y - (padding_y / 2.0f) - inner_padding_y) - 25.0f
        );
    }
}
void LobbyScreenFFA::update(const double& dt) {

    if (m_TimeLeftUntilMatchStarts > 0.0) {
        m_TimeLeftUntilMatchStarts -= dt;
        if (m_TimeLeftUntilMatchStarts < 0.0) {
            m_TimeLeftUntilMatchStarts = 0.0;


            
            m_Core.getClient()->getMapData().getMap().full_load();
            m_Ship3DViewer->hide();
            m_ChooseShipMessage->hide();

            auto& chosen_ship_class = getShipSelector().getChosenShipClass();

            PacketMessage p;
            p.PacketType = PacketType::Client_To_Server_Request_Map_Entry;
            p.name = m_Core.getClient()->username();

            p.data = (chosen_ship_class.empty()) ? "NULL" : chosen_ship_class; //ship class [0]

            p.data += "," + m_Core.getClient()->getMapData().getMap().name(); //map name [1]
            if (&m_Core.getClient()->getMapData().getTeam()) {
                p.data += "," + m_Core.getClient()->getMapData().getTeam().getTeamNumberAsString();
            }else{
                p.data += ",-1"; //error, the client did not choose or was not assigned a team yet
            }

            m_Core.getClient()->send(p);


        }
    }

    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackButton->update(dt);
    if (m_IsHost == true) {
        m_ForwardButton->update(dt);
    }
    m_RoundedWindow->update(dt);
    m_ShipSelector->update(dt);
    m_Ship3DViewer->update(dt);
    m_ShipDescription->update(dt);
    m_ChooseShipMessage->update(dt);
}
void LobbyScreenFFA::render() {
    const auto pos = m_RoundedWindow->positionWorld();

    const double mins_as_double = m_TimeLeftUntilMatchStarts / 60.0;
    const unsigned int mins = static_cast<unsigned int>(mins_as_double);
    const double remainder = mins_as_double - static_cast<double>(mins);
    const unsigned int secs = static_cast<unsigned int>(remainder * 60.0);

    string time_as_str = Helper::FormatTimeAsMinThenSecs(mins, secs);

    if (m_TimeLeftUntilMatchStarts > 5.9) {
        Renderer::renderText("Match starts: " + time_as_str, m_Font, glm::vec2(pos.x + (m_RoundedWindow->width() / 2.0f) - 260.0f, pos.y + (m_RoundedWindow->height() / 2.0f) - 12.0f), glm::vec4(0.35f, 0.35f, 0.35f, 1.0f), 0.0f, glm::vec2(0.8f), 0.001f, TextAlignment::Left);
    }else {
        Renderer::renderText("Match starts: " + time_as_str, m_Font, glm::vec2(pos.x + (m_RoundedWindow->width() / 2.0f) - 260.0f, pos.y + (m_RoundedWindow->height() / 2.0f) - 12.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, glm::vec2(0.8f), 0.001f, TextAlignment::Left);
    }

    m_BackgroundEdgeGraphicBottom->render();
    m_BackButton->render();
    if (m_IsHost == true) {
        m_ForwardButton->render();
    }
    m_RoundedWindow->render();
    m_ShipSelector->render();
    m_ShipDescription->render();
    m_ChooseShipMessage->render();
}
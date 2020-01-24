#include "HostScreen0.h"
#include "HostScreen1Persistent.h"
#include "OwnedServersSelectionWindow.h"
#include "../Button.h"
#include "../../Menu.h"
#include "../../database/Database.h"
#include "../../networking/server/Server.h"
#include "../../factions/Faction.h"

#include <core/engine/resources/Engine_Resources.h>


using namespace std;
using namespace Engine;

constexpr auto bottom_bar_height       = 50.0f;
constexpr auto bottom_bar_button_width = 150.0f;
constexpr auto bottom_bar_height_total = 80;

constexpr auto padding_x               = 100.0f;
constexpr auto padding_y               = 100.0f;

constexpr auto button_width            = 350.0f;
constexpr auto button_height           = 100.0f;
constexpr auto seperation_size         = 225.0f;

struct Host0_ButtonBack_OnClick final { void operator()(Button* button) const {
    auto& hostScreen0 = *static_cast<HostScreen0*>(button->getUserPointer());
    hostScreen0.m_Menu.go_to_main_menu();
}};

struct HostTempServClick final { void operator()(Button* button) const {
    auto& hostScreen0 = *static_cast<HostScreen0*>(button->getUserPointer());
    hostScreen0.m_Menu.setGameState(GameState::Host_Screen_Setup_1);
    hostScreen0.m_Menu.setErrorText("", 0.2f);
}};
struct HostPersServClick final { void operator()(Button* button) const {
    auto& hostScreen0 = *static_cast<HostScreen0*>(button->getUserPointer());

    Server::DATABASE.connect_to_database("database");

    hostScreen0.m_Menu.m_HostScreen1Persistent->m_OwnedServersWindow->populateWindow();
    hostScreen0.m_Menu.setGameState(GameState::Host_Screen_Setup_1_Persistent);
    hostScreen0.m_Menu.setErrorText("", 0.2f);
}};



HostScreen0::HostScreen0(Menu& menu, Font& font) : m_Font(font), m_Menu(menu) {
    const auto winSize = glm::vec2(Resources::getWindowSize());


    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, bottom_bar_height_total / 2.0f, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicBottom->setDepth(0.25f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host0_ButtonBack_OnClick());

    m_HostTemporaryServer = NEW Button(font, winSize.x / 2.0f, seperation_size, button_width, button_height);
    m_HostTemporaryServer->setText("Temporary Server");
    m_HostTemporaryServer->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_HostTemporaryServer->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_HostTemporaryServer->setTextScale(1.2f, 1.2f);
    m_HostTemporaryServer->setDepth(0.2f);
    m_HostTemporaryServer->setUserPointer(this);
    m_HostTemporaryServer->setOnClickFunctor(HostTempServClick());

    m_HostPersistentServer = NEW Button(font, winSize.x / 2.0f, -seperation_size, button_width, button_height);
    m_HostPersistentServer->setText("Persistent Server");
    m_HostPersistentServer->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_HostPersistentServer->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_HostPersistentServer->setTextScale(1.2f, 1.2f);
    m_HostPersistentServer->setDepth(0.2f);
    m_HostPersistentServer->setUserPointer(this);
    m_HostPersistentServer->setOnClickFunctor(HostPersServClick());
}
HostScreen0::~HostScreen0() {
    SAFE_DELETE(m_HostTemporaryServer);
    SAFE_DELETE(m_HostPersistentServer);
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
}

Button& HostScreen0::getHostTemporaryServerButton() {
    return *m_HostTemporaryServer;
}
Button& HostScreen0::getHostPersistentServerButton() {
    return *m_HostPersistentServer;
}

void HostScreen0::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::vec2(newWidth, newHeight);

    m_HostTemporaryServer->setPosition(
        (newWidth / 2.0f), 
        (newHeight / 2.0f) + (seperation_size / 2.0f)
    );
    m_HostPersistentServer->setPosition(
        (newWidth / 2.0f),
        (newHeight / 2.0f) - (seperation_size / 2.0f)
    );

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);
}

void HostScreen0::update(const double& dt) {
    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackButton->update(dt);

    m_HostTemporaryServer->update(dt);
    m_HostPersistentServer->update(dt);
}
void HostScreen0::render() {
    m_BackgroundEdgeGraphicBottom->render();
    m_BackButton->render();

    m_HostTemporaryServer->render();
    m_HostPersistentServer->render();
}
#include "HostScreen1Persistent.h"
#include "HostScreen1.h"
#include "OwnedServersSelectionWindow.h"
#include "CreateServerWindow.h"

#include <core/engine/resources/Engine_Resources.h>

#include "../../factions/Faction.h"
#include "../../Menu.h"

#include "../Button.h"

using namespace std;
using namespace Engine;

constexpr auto padding_x = 100.0f;
constexpr auto padding_y = 100.0f;
constexpr auto bottom_bar_height = 50.0f;
constexpr auto bottom_bar_button_width = 150.0f;

constexpr auto bottom_bar_height_total = 80;

constexpr auto left_window_width = 800;
constexpr auto right_window_width = 550;

struct Host1Persistent_ButtonBack_OnClick final { void operator()(Button* button) const {
    auto& hostScreen1Persistent = *static_cast<HostScreen1Persistent*>(button->getUserPointer());
    hostScreen1Persistent.m_Menu.setGameState(GameState::Host_Screen_Setup_0);
    hostScreen1Persistent.m_Menu.setErrorText("", 0.2f);
};};
struct Host1Persistent_ButtonNext_OnClick final { void operator()(Button* button) const {
    auto& hostScreen1Persistent = *static_cast<HostScreen1Persistent*>(button->getUserPointer());

    /*
        take the chosen server you made and go to HostScreen1 as persistent
    */
    hostScreen1Persistent.m_Menu.m_HostScreen1->setPersistent();
    hostScreen1Persistent.m_Menu.setGameState(GameState::Host_Screen_Setup_1);

    hostScreen1Persistent.m_Menu.setErrorText("", 0.2f);
};};


HostScreen1Persistent::HostScreen1Persistent(Menu& menu, Font& font) : m_Menu(menu), m_Font(font){
    const auto winSize = glm::vec2(Resources::getWindowSize());

    m_BackgroundEdgeGraphicBottom = NEW Button(font, winSize.x / 2.0f, bottom_bar_height_total / 2.0f, winSize.x, bottom_bar_height_total);
    m_BackgroundEdgeGraphicBottom->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_BackgroundEdgeGraphicBottom->setDepth(0.512f);
    m_BackgroundEdgeGraphicBottom->disable();
    m_BackgroundEdgeGraphicBottom->setTextureCorner(nullptr);
    m_BackgroundEdgeGraphicBottom->enableTextureCorner(false);

    m_BackButton = NEW Button(font, padding_x + (bottom_bar_button_width / 2.0f), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_BackButton->setText("Back");
    m_BackButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_BackButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_BackButton->setUserPointer(this);
    m_BackButton->setOnClickFunctor(Host1Persistent_ButtonBack_OnClick());

    m_ForwardButton = NEW Button(font, winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), padding_y + (bottom_bar_height / 2.0f), bottom_bar_button_width, bottom_bar_height);
    m_ForwardButton->setText("Next");
    m_ForwardButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ForwardButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ForwardButton->setUserPointer(this);
    m_ForwardButton->setOnClickFunctor(Host1Persistent_ButtonNext_OnClick());


    const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
    {
        m_OwnedServersWindow = NEW OwnedServersSelectionWindow(*this, font,
            (padding_x / 2.0f) + (left_window_width / 2.0f),
            winSize.y - (padding_y / 2.0f) - (window_height / 2.0f),
            left_window_width,
            window_height,
            0.05f, 1, "Your Servers");
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
        m_OwnedServersWindow->setPositionFunctor(LeftPositionFunctor());
        m_OwnedServersWindow->setSizeFunctor(LeftSizeFunctor());

    }
    {
        m_CreateServersWindow = NEW CreateServerWindow(*this, font,
            winSize.x - (padding_x / 2.0f) - (window_height / 2.0f),
            winSize.y - (padding_y / 2.0f) - (window_height / 2.0f),
            right_window_width,
            window_height,
            0.035f, 1, "Create Server");
        struct RightSizeFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            return glm::vec2(right_window_width, window_height);
        }};
        struct RightPositionFunctor { glm::vec2 operator()(RoundedWindow* window) const {
            const auto winSize = Resources::getWindowSize();
            const auto window_height = (winSize.y - bottom_bar_height_total - padding_y);
            const auto x = winSize.x - (padding_x / 2.0f) - (right_window_width / 2.0f);
            const auto y = winSize.y - (padding_y / 2.0f) - (window_height / 2.0f);
            return glm::vec2(x, y);
        }};
        m_CreateServersWindow->setPositionFunctor(RightPositionFunctor());
        m_CreateServersWindow->setSizeFunctor(RightSizeFunctor());
    }

}
HostScreen1Persistent::~HostScreen1Persistent() {
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_ForwardButton);

    SAFE_DELETE(m_OwnedServersWindow);
    SAFE_DELETE(m_CreateServersWindow);
}

void HostScreen1Persistent::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    const auto winSize = glm::vec2(newWidth, newHeight);

    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_ForwardButton->setPosition(winSize.x - (padding_x + (bottom_bar_button_width / 2.0f)), bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);

    m_OwnedServersWindow->onResize(newWidth, newHeight);
    m_CreateServersWindow->onResize(newWidth, newHeight);
}

void HostScreen1Persistent::update(const double& dt) {
    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackButton->update(dt);
    m_ForwardButton->update(dt);

    m_OwnedServersWindow->update(dt);
    m_CreateServersWindow->update(dt);
}
void HostScreen1Persistent::render() {
    m_BackgroundEdgeGraphicBottom->render();
    m_BackButton->render();
    m_ForwardButton->render();

    m_OwnedServersWindow->render();
    m_CreateServersWindow->render();
}
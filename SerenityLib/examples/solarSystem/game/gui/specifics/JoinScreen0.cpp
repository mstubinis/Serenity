#include "JoinScreen0.h"

#include "../../Menu.h"
#include "../Button.h"
#include "../../factions/Faction.h"

using namespace std;
using namespace Engine;

constexpr auto bottom_bar_height = 50.0f;
constexpr auto bottom_bar_button_width = 150.0f;
constexpr auto bottom_bar_height_total = 80;

constexpr auto padding_x = 100.0f;
constexpr auto padding_y = 100.0f;

constexpr auto button_width = 350.0f;
constexpr auto button_height = 100.0f;

struct Join0_ButtonBack_OnClick final { void operator()(Button* button) const {
    auto& joinScreen0 = *static_cast<JoinScreen0*>(button->getUserPointer());
    joinScreen0.m_Menu.go_to_main_menu();
}};

JoinScreen0::JoinScreen0(Menu& menu, Font& font) : m_Menu(menu), m_Font(font) {
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
    m_BackButton->setOnClickFunctor(Join0_ButtonBack_OnClick());
}
JoinScreen0::~JoinScreen0() {
    SAFE_DELETE(m_BackButton);
    SAFE_DELETE(m_BackgroundEdgeGraphicBottom);
}

void JoinScreen0::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::vec2(newWidth, newHeight);



    m_BackButton->setPosition(padding_x + (bottom_bar_button_width / 2.0f), bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setPosition(winSize.x / 2.0f, bottom_bar_height_total / 2.0f);
    m_BackgroundEdgeGraphicBottom->setSize(winSize.x, bottom_bar_height_total);
}

void JoinScreen0::update(const double& dt) {
    m_BackgroundEdgeGraphicBottom->update(dt);
    m_BackButton->update(dt);


}
void JoinScreen0::render() {
    m_BackgroundEdgeGraphicBottom->render();
    m_BackButton->render();


}
#include "MainMenu.h"
#include "../Button.h"
#include "../../Menu.h"
#include "../../factions/Faction.h"

#include <core/engine/system/Engine.h>

using namespace Engine;
using namespace std;

struct Button_Host_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Host_Screen_Setup_0);
    menu.setErrorText("", 0.2f);
}};
struct Button_Join_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Join_Screen_Setup_1);
    menu.setErrorText("", 0.2f);
}};

struct Button_Options_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Options_Main);
    menu.setErrorText("", 0.2f);
}};
struct Button_Encyclopedia_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Encyclopedia_Main);
    menu.setErrorText("", 0.2f);
}};
struct Button_Exit_OnClick { void operator()(Button* button) const {
    Engine::stop();
}};

MainMenu::MainMenu(Menu& menu, Font& font, const float& depth) {
    const auto windowDimensions = Resources::getWindowSize();

    const auto padding_y = -70.0f;
    const auto button_width = 210.0f;
    const auto button_height = 50.0f;

    m_ButtonHost = NEW Button(font, windowDimensions.x / 2.0f, 365.0f, button_width, button_height);
    m_ButtonHost->setText("Host");
    m_ButtonHost->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ButtonHost->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ButtonHost->setDepth(depth);

    m_ButtonsBackground = NEW Button(font, 0, 40.0f, (button_width * 1.5f), ((button_height + 22.0f) * 5));
    m_ButtonsBackground->setAlignment(Alignment::TopCenter);
    m_ButtonsBackground->disable();
    m_ButtonsBackground->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    m_ButtonsBackground->setDepth(depth + 0.01f);

    m_ButtonsBackgroundBorder = NEW Button(font, 0, 41.0f, (button_width * 1.5f)+2, ((button_height + 22.0f) * 5)+2);
    m_ButtonsBackgroundBorder->setAlignment(Alignment::TopCenter);
    m_ButtonsBackgroundBorder->disable();
    m_ButtonsBackgroundBorder->setColor(Factions::Database[FactionEnum::Federation].GUIColorSlightlyDarker);
    m_ButtonsBackgroundBorder->setDepth(depth + 0.02f);

    m_ButtonJoin = NEW Button(font, 0.0f, padding_y, button_width, button_height);
    m_ButtonJoin->setText("Join");
    m_ButtonJoin->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ButtonJoin->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ButtonJoin->setDepth(depth);

    m_ButtonOptions = NEW Button(font, 0.0f, padding_y * 2, button_width, button_height);
    m_ButtonOptions->setText("Options");
    m_ButtonOptions->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ButtonOptions->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ButtonOptions->setDepth(depth);

    m_ButtonEncyclopedia = NEW Button(font, 0.0f, padding_y * 3, button_width, button_height);
    m_ButtonEncyclopedia->setText("Encyclopedia");
    m_ButtonEncyclopedia->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ButtonEncyclopedia->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ButtonEncyclopedia->setDepth(depth);

    m_ButtonExit = NEW Button(font, 0.0f, padding_y * 4, button_width, button_height);
    m_ButtonExit->setText("Exit");
    m_ButtonExit->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_ButtonExit->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_ButtonExit->setDepth(depth);

    //TODO: get rid of these when these pages are implemented
    m_ButtonOptions->hide();
    m_ButtonEncyclopedia->hide();
    //m_ButtonExit->hide();

    m_ButtonHost->addChild(m_ButtonsBackground);
    m_ButtonHost->addChild(m_ButtonsBackgroundBorder);
    m_ButtonHost->addChild(m_ButtonJoin);
    m_ButtonHost->addChild(m_ButtonOptions);
    m_ButtonHost->addChild(m_ButtonEncyclopedia);
    m_ButtonHost->addChild(m_ButtonExit);

    m_ButtonHost->setUserPointer(&menu);
    m_ButtonJoin->setUserPointer(&menu);
    m_ButtonOptions->setUserPointer(&menu);
    m_ButtonEncyclopedia->setUserPointer(&menu);
    m_ButtonExit->setUserPointer(&menu);

    m_ButtonHost->setOnClickFunctor(Button_Host_OnClick());
    m_ButtonJoin->setOnClickFunctor(Button_Join_OnClick());
    m_ButtonOptions->setOnClickFunctor(Button_Options_OnClick());
    m_ButtonEncyclopedia->setOnClickFunctor(Button_Encyclopedia_OnClick());
    m_ButtonExit->setOnClickFunctor(Button_Exit_OnClick());
}
MainMenu::~MainMenu() {
    SAFE_DELETE(m_ButtonHost);
}

void MainMenu::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    m_ButtonHost->setPosition(newWidth / 2.0f, 365.0f);
}

void MainMenu::update(const double& dt) {
    m_ButtonHost->update(dt);
}
void MainMenu::render() {
    m_ButtonHost->render();
}
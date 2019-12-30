#include "MainMenu.h"
#include "../Button.h"
#include "../../Menu.h"

using namespace Engine;
using namespace std;

struct Button_Host_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Host_Server_Port_And_Name_And_Map);
    menu.setErrorText("", 0.2f);
}};
struct Button_Join_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Join_Server_Port_And_Name_And_IP);
    menu.setErrorText("", 0.2f);
}};

struct Button_Options_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Options);
    menu.setErrorText("", 0.2f);
}};
struct Button_Encyclopedia_OnClick { void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    menu.setGameState(GameState::Encyclopedia);
    menu.setErrorText("", 0.2f);
}};


MainMenu::MainMenu(Menu& menu, Font& font) {
    const auto windowDimensions = Resources::getWindowSize();

    const auto padding_y = -90.0f;
    const auto button_width = 210.0f;
    const auto button_height = 50.0f;

    m_ButtonHost = NEW Button(font, windowDimensions.x / 2.0f, 345.0f, button_width, button_height);
    m_ButtonHost->setText("Host");
    m_ButtonHost->setColor(   Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]  );
    m_ButtonHost->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);



    m_ButtonJoin = NEW Button(font, 0.0f, padding_y, button_width, button_height);
    m_ButtonJoin->setText("Join");
    m_ButtonJoin->setColor(  Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]  );
    m_ButtonJoin->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_ButtonOptions = NEW Button(font, 0.0f, padding_y * 2, button_width, button_height);
    m_ButtonOptions->setText("Options");
    m_ButtonOptions->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_ButtonOptions->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_ButtonEncyclopedia = NEW Button(font, 0.0f, padding_y * 3, button_width, button_height);
    m_ButtonEncyclopedia->setText("Encyclopedia");
    m_ButtonEncyclopedia->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_ButtonEncyclopedia->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);


    //TODO: get rid of these when these pages are implemented
    m_ButtonOptions->hide();
    m_ButtonEncyclopedia->hide();

    m_ButtonHost->addChild(m_ButtonJoin);
    m_ButtonHost->addChild(m_ButtonOptions);
    m_ButtonHost->addChild(m_ButtonEncyclopedia);

    m_ButtonHost->setUserPointer(&menu);
    m_ButtonJoin->setUserPointer(&menu);
    m_ButtonOptions->setUserPointer(&menu);
    m_ButtonEncyclopedia->setUserPointer(&menu);

    m_ButtonHost->setOnClickFunctor(Button_Host_OnClick());
    m_ButtonJoin->setOnClickFunctor(Button_Join_OnClick());
    m_ButtonOptions->setOnClickFunctor(Button_Options_OnClick());
    m_ButtonEncyclopedia->setOnClickFunctor(Button_Encyclopedia_OnClick());
}
MainMenu::~MainMenu() {
    SAFE_DELETE(m_ButtonHost);
}

void MainMenu::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    m_ButtonHost->setPosition(newWidth / 2.0f, 345.0f);
}

void MainMenu::update(const double& dt) {
    m_ButtonHost->update(dt);
}
void MainMenu::render() {
    m_ButtonHost->render();
}
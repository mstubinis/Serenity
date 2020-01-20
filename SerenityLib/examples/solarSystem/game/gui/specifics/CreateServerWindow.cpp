#include "CreateServerWindow.h"
#include "HostScreen1Persistent.h"
#include "../../factions/Faction.h"
#include "../Text.h"
#include "../TextBox.h"
#include "../ScrollFrame.h"
#include "../../config/ConfigFile.h"
#include "../../database/Database.h"

using namespace std;

constexpr auto scroll_frame_padding = 30.0f;

struct ServerCreateOnClick final { void operator()(Button* button) {
    auto& hostScreen1Persistent = *static_cast<HostScreen1Persistent*>(button->getUserPointer());
    /*
        add a new server using the input provided. check / sanitize the input
    */



    hostScreen1Persistent.m_CreateServersWindow->resetWindow();
};};

CreateServerWindow::CreateServerWindow(HostScreen1Persistent& hostScreen1Persistent, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font, x, y, width, height, depth, borderSize, labelText), m_Font(font), m_HostScreen1Persistent(hostScreen1Persistent) {

    m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);

    m_ScrollFrame = NEW ScrollFrame(font, x, y - (height / 2.0f) + scroll_frame_padding, width - (scroll_frame_padding * 2.0f), height - 110.0f, depth);
    m_ScrollFrame->setAlignment(Alignment::BottomCenter);
    m_ScrollFrame->setPaddingSize(15, 2);
    m_ScrollFrame->setPaddingSize(15, 3);
    m_ScrollFrame->setPaddingSize(15, 1);

    const auto txt_scale = 0.72f;
    const auto padding = 0;

    auto* server_name = NEW TextBox("Server name  ", font, Database::CONST_SERVER_NAME_LENGTH_MAX, x, y);
    server_name->setAlignment(Alignment::TopLeft);
    server_name->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    server_name->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    server_name->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    server_name->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    server_name->setTextScale(txt_scale, txt_scale);
    server_name->setText("");
    server_name->setPaddingSize(padding);
    server_name->setBorderSize(1);


    auto* server_port = NEW TextBox("Server port  ", font, 6, x, y);
    server_port->setAlignment(Alignment::TopLeft);
    server_port->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    server_port->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    server_port->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    server_port->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    server_port->setText(to_string(ConfigFile::CONFIG_DATA.host_server_port));
    server_port->setTextScale(txt_scale, txt_scale);
    server_port->setPaddingSize(padding);
    server_port->setBorderSize(1);


    auto* username = NEW TextBox("Username  ", font, Database::CONST_USERNAME_LENGTH_MAX, x, y);
    username->setAlignment(Alignment::TopLeft);
    username->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    username->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    username->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    username->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    username->setText(ConfigFile::CONFIG_DATA.host_server_player_name);
    username->setTextScale(txt_scale, txt_scale);
    username->setPaddingSize(padding);
    username->setBorderSize(1);

    auto* password = NEW TextBox("Password  ", font, Database::CONST_USERNAME_PASSWORD_LENGTH_MAX, x, y);
    password->setTextDisplayMode(TextBox::TextDisplayMode::PasswordRevealLastChar);
    password->setAlignment(Alignment::TopLeft);
    password->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    password->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    password->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    password->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    password->setText("");
    password->setTextScale(txt_scale, txt_scale);
    password->setPaddingSize(padding);
    password->setBorderSize(1);


    m_ScrollFrame->addContent(server_name, 0);
    m_ScrollFrame->addContent(server_port, 1);
    m_ScrollFrame->addContent(username, 2);
    m_ScrollFrame->addContent(password, 3);

    m_CreateButton = new Button(font, x,y - (height / 2.0f) + 160, 170, 50);
    m_CreateButton->setText("Create");
    m_CreateButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_CreateButton->setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_CreateButton->setDepth(depth);
    m_CreateButton->setUserPointer(&hostScreen1Persistent);
    m_CreateButton->setOnClickFunctor(ServerCreateOnClick());
}
CreateServerWindow::~CreateServerWindow() {
    SAFE_DELETE(m_ScrollFrame);
    SAFE_DELETE(m_CreateButton);
}

TextBox& CreateServerWindow::getServerNameTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[0].widgets[0].widget);
}
TextBox& CreateServerWindow::getServerPortTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[1].widgets[0].widget);
}
TextBox& CreateServerWindow::getUsernameTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[2].widgets[0].widget);
}
TextBox& CreateServerWindow::getPasswordTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[3].widgets[0].widget);
}

void CreateServerWindow::resetWindow() {
    getServerNameTextBox().setText("");
    getUsernameTextBox().setText(ConfigFile::CONFIG_DATA.host_server_player_name);
    getPasswordTextBox().setText("");
}

void CreateServerWindow::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    m_ScrollFrame->setPosition(
        pos.x,
        pos.y - (frame_size.y / 2.0f) + scroll_frame_padding
    );
    m_ScrollFrame->setSize(
        frame_size.x - (scroll_frame_padding * 2.0f),
        frame_size.y - 110.0f
    );

    m_CreateButton->setPosition(pos.x, pos.y - (newHeight / 2.0f) + 160);
}

void CreateServerWindow::update(const double& dt) {
    RoundedWindow::update(dt);

    m_ScrollFrame->update(dt);
    m_CreateButton->update(dt);
}
void CreateServerWindow::render() {
    RoundedWindow::render();

    m_ScrollFrame->render();
    m_CreateButton->render();
}
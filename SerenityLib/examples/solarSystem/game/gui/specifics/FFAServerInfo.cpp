#include "FFAServerInfo.h"
#include "HostScreenFFA2.h"
#include "../TextBox.h"
#include "../../factions/Faction.h"
#include "../../map/Map.h"
#include "../../Menu.h"
#include "../../Core.h"
#include "../Text.h"
#include "../ScrollFrame.h"
#include "../../ships/Ships.h"
#include "../../networking/server/Server.h"
#include "../../config/ConfigFile.h"
#include "../../database/Database.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace std;
using namespace Engine;

constexpr auto scroll_frame_padding = 30.0f;

FFAServerInfo::FFAServerInfo(HostScreenFFA2& hostScreen, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font, x, y, width, height, depth, borderSize, labelText), m_HostScreen(hostScreen), m_Font(font){

    m_IsPersistent = false;
    m_ScrollFrame           = NEW ScrollFrame(font, x, y - (height / 2.0f) + scroll_frame_padding, width - (scroll_frame_padding * 2.0f), height - 110.0f, depth);
    m_ScrollFrame->setAlignment(Alignment::BottomCenter);
    m_ScrollFrame->setPaddingSize(15, 2);
    m_ScrollFrame->setPaddingSize(15, 3);
    m_ScrollFrame->setPaddingSize(15, 1);

    const auto txt_scale = 0.72f;
    const auto padding = 0;

    auto* your_name = NEW TextBox("Your name  ", font, Database::CONST_USERNAME_LENGTH_MAX, x, y);
    your_name->setAlignment(Alignment::TopLeft);
    your_name->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    your_name->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    your_name->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    your_name->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    your_name->setTextScale(txt_scale, txt_scale);
    your_name->setText(ConfigFile::CONFIG_DATA.host_server_player_name);
    your_name->setPaddingSize(padding);
    your_name->setBorderSize(1);
    

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


    auto* max_players = NEW TextBox("Max players  ", font, 2, x, y);
    max_players->setAlignment(Alignment::TopLeft);
    max_players->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    max_players->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    max_players->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    max_players->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    max_players->setText("15");
    max_players->setTextScale(txt_scale, txt_scale);
    max_players->setPaddingSize(padding);
    max_players->setBorderSize(1);

    auto* match_duration = NEW TextBox("Match duration  ", font, 5, x, y);
    match_duration->setAlignment(Alignment::TopLeft);
    match_duration->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    match_duration->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    match_duration->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    match_duration->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    match_duration->setText("15:00");
    match_duration->setTextScale(txt_scale, txt_scale);
    match_duration->setPaddingSize(padding);
    match_duration->setBorderSize(1);

    auto* lobby_duration = NEW TextBox("Lobby duration  ", font, 5, x, y);
    lobby_duration->setAlignment(Alignment::TopLeft);
    lobby_duration->setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    lobby_duration->setBorderColor(Factions::Database[FactionEnum::Federation].GUIColor);
    lobby_duration->setTextColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    lobby_duration->setLabelTextColor(Factions::Database[FactionEnum::Federation].GUIColor);
    lobby_duration->setText("10:00");
    lobby_duration->setTextScale(txt_scale, txt_scale);
    lobby_duration->setPaddingSize(padding);
    lobby_duration->setBorderSize(1);

    m_ScrollFrame->addContent(your_name, 0);
    m_ScrollFrame->addContent(server_port, 1);
    m_ScrollFrame->addContent(max_players, 1);
    m_ScrollFrame->addContent(match_duration, 2);
    m_ScrollFrame->addContent(lobby_duration, 3);

    m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
}
FFAServerInfo::~FFAServerInfo() {
    SAFE_DELETE(m_ScrollFrame);
}
const bool FFAServerInfo::isPersistent() const {
    return m_IsPersistent;
}
void FFAServerInfo::setPersistent() {
    m_IsPersistent = true;

    TextBox& your_name = getYourNameTextBox();
    TextBox& server_port = getServerPortTextBox();

    //TODO: fill info via database query
    //auto res = hostScreen.getMenu().getCore().getServer()->getDatabase().execute_query();

    your_name.lock();
    server_port.lock();
}
TextBox& FFAServerInfo::getYourNameTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[0].widgets[0].widget);
}
TextBox& FFAServerInfo::getServerPortTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[1].widgets[0].widget);
}
TextBox& FFAServerInfo::getMaxPlayersTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[1].widgets[1].widget);
}
TextBox& FFAServerInfo::getMatchDurationTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[2].widgets[0].widget);
}
TextBox& FFAServerInfo::getLobbyDurationTextBox() {
    return *static_cast<TextBox*>(m_ScrollFrame->content()[3].widgets[0].widget);
}
void FFAServerInfo::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
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

}
void FFAServerInfo::update(const double& dt) {
    RoundedWindow::update(dt);

    m_ScrollFrame->update(dt);
}
void FFAServerInfo::render() {
    RoundedWindow::render();

    m_ScrollFrame->render();
}
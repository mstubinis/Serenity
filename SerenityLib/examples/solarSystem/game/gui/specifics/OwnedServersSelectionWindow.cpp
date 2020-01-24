#include "OwnedServersSelectionWindow.h"
#include "../Text.h"
#include "../Button.h"
#include "../ScrollFrame.h"
#include "../../factions/Faction.h"
#include "../../database/Database.h"
#include "../../networking/server/Server.h"

#include <core/engine/renderer/Renderer.h>

using namespace std;

constexpr auto scroll_frame_padding = 30.0f;

struct ServerButtonOnClick final { void operator()(Button* button) const {
    auto& ptr = *static_cast<OwnedServersSelectionWindow::UserPointer*>(button->getUserPointer());

    ptr.window->clearSelectedServer();
    ptr.window->m_SelectedServer = &ptr;
    button->setColor(Factions::Database[FactionEnum::Federation].GUIColorHighlight);
    button->disableMouseover();

}};

OwnedServersSelectionWindow::OwnedServersSelectionWindow(HostScreen1Persistent& hostScreen1Persistent, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font,x,y, width, height, depth,borderSize,labelText), m_Font(font), m_HostScreen1Persistent(hostScreen1Persistent){
    m_SelectedServer = nullptr;
    m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);

    m_ScrollFrame = NEW ScrollFrame(font, 
        x, 
        y, 
        (width) - (scroll_frame_padding * 2.0f), 
        height - 240.0f, 
    depth - 0.001f);
}
OwnedServersSelectionWindow::~OwnedServersSelectionWindow() {
    clearWindow();
    SAFE_DELETE(m_ScrollFrame);
}
OwnedServersSelectionWindow::UserPointer* OwnedServersSelectionWindow::getSelectedServer() {
    return m_SelectedServer;
}
const unsigned int OwnedServersSelectionWindow::getNumServers() {
    unsigned int count = 0;
    for (auto& widget_row : m_ScrollFrame->content()) {
        for (auto& widget_ptr : widget_row.widgets) {
            ++count;
        }
    }
    return count;
}
const bool OwnedServersSelectionWindow::hasServer(const string& serverName) {
    for (auto& widget_row : m_ScrollFrame->content()) {
        for (auto& widget_ptr : widget_row.widgets) {
            Button& b = *static_cast<Button*>(widget_ptr.widget);
            if (b.text() == serverName) {
                return true;
            }
        }
    }
    return false;
}
void OwnedServersSelectionWindow::populateWindow() {
    clearWindow();

    sqlite3_stmt* statement;
    Server::DATABASE.prepare_sql_statement("SELECT * FROM STHS_ServerList ORDER BY Name ASC", &statement);
    auto results = Server::DATABASE.execute_query_and_return_results(statement);  
    size_t i = 0;
    for (auto& row : results) {
        const auto server_id    = results.get(i, "ServerID");
        const auto server_name  = results.get(i, "Name");
        const auto server_port  = results.get(i, "Port");
        const auto server_owner = results.get(i, "OwnerUsername");
        add_button(server_name, stoi(server_port), server_owner);
        ++i;
    }
    
}
void OwnedServersSelectionWindow::clearSelectedServer() {
    for (auto& row : m_ScrollFrame->content()) {
        for (auto& widget_ptr : row.widgets) {
            auto& button = *static_cast<Button*>(widget_ptr.widget);
            button.enableMouseover();
            button.setColor(Factions::Database[FactionEnum::Federation].GUIColor);
        }
    }
    m_SelectedServer = nullptr;
}
void OwnedServersSelectionWindow::clearWindow() {
    for (auto& widget_row : m_ScrollFrame->content()) {
        for (auto& widget_ptr : widget_row.widgets) {
            auto& button = *static_cast<Button*>(widget_ptr.widget);
            auto* ptr = static_cast<OwnedServersSelectionWindow::UserPointer*>(button.getUserPointer());
            SAFE_DELETE(ptr);
        }
    }
    m_ScrollFrame->clear();
}
void OwnedServersSelectionWindow::add_button(const string& serverName, const unsigned int& serverPort, const string& username) {
    Button& button = *NEW Button(m_Font, 0.0f, 0.0f, (m_ScrollFrame->width() ), 180.0f);
    button.setText(serverName);
    button.setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    button.setTextColor(0.0f, 0.0f, 0.0f, 1.0f);
    button.setAlignment(Alignment::TopLeft);
    button.setTextAlignment(TextAlignment::Left);

    auto* ptr = NEW OwnedServersSelectionWindow::UserPointer();
    ptr->serverName = serverName;
    ptr->window     = this;
    ptr->port       = serverPort;
    ptr->ownerName  = username;

    button.setUserPointer(ptr);
    button.setOnClickFunctor(ServerButtonOnClick());
    button.setPaddingSize(40);

    m_ScrollFrame->addContent(&button);
}
const bool OwnedServersSelectionWindow::addServer(const string& serverName, const unsigned int& serverPort, const string& username, const string& password) {
    
    const bool res = Server::DATABASE.create_new_server(serverName, serverPort, username, password);

    if (res) {
        populateWindow();
        return true;
    }
    return false;
}

void OwnedServersSelectionWindow::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    m_ScrollFrame->setPosition(
        pos.x - (frame_size.x / 2.0f) + scroll_frame_padding,
        pos.y + (frame_size.y / 2.0f) - 60.0f
    );
    m_ScrollFrame->setSize(
        frame_size.x - (scroll_frame_padding * 2.0f),
        frame_size.y - 240.0f
    );
}

void OwnedServersSelectionWindow::update(const double& dt) {
    RoundedWindow::update(dt);

    m_ScrollFrame->update(dt);
}
void OwnedServersSelectionWindow::render() {
    RoundedWindow::render();

    m_ScrollFrame->render();

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    if (m_SelectedServer) {
        string text = "Server: " + m_SelectedServer->serverName + "\n\nPort: " + to_string(m_SelectedServer->port) + "\n\nOwner: " + m_SelectedServer->ownerName;
        Engine::Renderer::renderText(text, m_Font, glm::vec2(pos.x - (frame_size.x / 2.0f) + scroll_frame_padding + 60.0f, pos.y - (frame_size.y / 2.0f) + 160 ), Factions::Database[FactionEnum::Federation].GUIColor, 0.0f, glm::vec2(0.7f), 0.01f);
    }
}
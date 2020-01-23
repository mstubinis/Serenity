#include "LoadingScreen.h"

#include <core/engine/resources/Engine_Resources.h>

#include "../../map/Map.h"
#include "../../Menu.h"
#include "../../Core.h"
#include "../../networking/client/Client.h"
#include "../../networking/client/ClientMapSpecificData.h"
#include "../../teams/Team.h"

#include "../../networking/packets/PacketMessage.h"
#include "../../factions/Faction.h"
#include "../../ships/Ships.h"

#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/threading/Engine_ThreadManager.h>

using namespace std;
using namespace Engine;

LoadingScreen::LoadingScreen(Menu& menu, Font& font) : m_Font(font), m_Menu(menu) {
    m_ShipClass  = "";
    m_SentPacket = false;
    m_Progress   = 0.0f;
}
LoadingScreen::~LoadingScreen() {
    if (m_Thread.joinable())
        m_Thread.join();
}

const bool LoadingScreen::setShipClass(const string& ship_class) {
    if (m_ShipClass == ship_class)
        return false;
    if (!Ships::Database.count(ship_class)) {
        return false;
    }
    m_ShipClass = ship_class;
    return true;
}


const bool LoadingScreen::startLoadingProcess() {
    auto& client = *m_Menu.getCore().getClient();

    if (client.getMapData() == false)
        return false;
    auto& map = client.getMapData().getMap();


    auto lamda = [this, &map]() -> void {
        const_cast<Map&>(map).full_load(this);
    };

    //m_Thread = std::thread(lamda);
    lamda();
    return true;
}
void LoadingScreen::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {

}

void LoadingScreen::setProgress(const float progress) {
    m_Progress = progress;
}

void LoadingScreen::update(const double& dt) {
    auto& client = *m_Menu.getCore().getClient();
    const auto& map = client.getMapData().getMap();
    if (map.isFullyLoaded()) {
        if (!m_SentPacket) {
            PacketMessage p;
            p.PacketType = PacketType::Client_To_Server_Request_Map_Entry;
            p.name = client.username();

            p.data = (m_ShipClass.empty()) ? "NULL" : m_ShipClass; //ship class [0]

            p.data += "," + client.getMapData().getMap().name(); //map name [1]
            if (&client.getMapData().getTeam()) {
                p.data += "," + client.getMapData().getTeam().getTeamNumberAsString();
            }else{
                p.data += ",-1"; //error, the client did not choose or was not assigned a team yet
            }
            client.send(p);
            m_SentPacket = true;
        }
    }
}
void LoadingScreen::render() {
    const glm::vec2 winSize = Engine::Resources::getWindowSize();
    const unsigned int percent = static_cast<unsigned int>(m_Progress * 100.0f);
    if (!m_ShipClass.empty()) {
        auto& screenshot_handle = Ships::Database[m_ShipClass].ScreenshotLoadingScreenHandle;
        if (!screenshot_handle.null()) {
            Texture& ss_texture = *(Texture*)screenshot_handle.get();
            Engine::Renderer::renderTexture(ss_texture, glm::vec2(
                (winSize.x / 2.0f) - (ss_texture.width() / 2.0f) - 20.0f,
                winSize.y / 2.0f), 
            glm::vec4(1.0f), 0.0f, glm::vec2(1.0f), 0.004f);
        }
    }
    Engine::Renderer::renderText(to_string(percent) + "%", m_Font, glm::vec2(
        (winSize.x / 2.0f), 
        (winSize.y / 2.0f) - 250.0f), 
    Factions::Database[FactionEnum::Federation].GUIColorText1, 0.0f, glm::vec2(1.0f), 0.005f);
}
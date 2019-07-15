#include "HUD.h"
#include "SolarSystem.h"
#include "Planet.h"
#include "Ship.h"
#include "Core.h"
#include "ResourceManifest.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/Engine_Math.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/textures/Texture.h>
#include <core/Scene.h>
#include <core/Camera.h>
#include <core/Material.h>

#include <glm/vec4.hpp>

#include "gui/Button.h"

using namespace Engine;
using namespace std;


struct ButtonHost_OnClick { void operator()(Button& button) const {
    HUD& hud = *static_cast<HUD*>(button.getUserPointer());
    hud.m_GameState = GameState::Host_Server_Port_And_Name;
}};
struct ButtonJoin_OnClick {void operator()(Button& button) const {
    HUD& hud = *static_cast<HUD*>(button.getUserPointer());
    hud.m_GameState = GameState::Join_Server_Port_And_Name_And_IP;
}};


struct ButtonBack_OnClick {void operator()(Button& button) const {
    HUD& hud = *static_cast<HUD*>(button.getUserPointer());
    switch (hud.m_GameState) {
        case GameState::Host_Server_Port_And_Name:{
            hud.m_GameState = GameState::Main_Menu;
            hud.m_Next->setText("Next");
            break;
        }case GameState::Host_Server_Map_And_Ship: {
            hud.m_Core.shutdownClient();
            hud.m_Core.shutdownServer();
            hud.m_GameState = GameState::Host_Server_Port_And_Name;
            hud.m_Next->setText("Next");
            break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            hud.m_GameState = GameState::Main_Menu;
            hud.m_Next->setText("Next");
            break;
        }case GameState::Join_Server_Server_Info: {
            hud.m_GameState = GameState::Join_Server_Port_And_Name_And_IP;
            hud.m_Next->setText("Next");
            break;
        }default: {
            hud.m_GameState = GameState::Main_Menu;
            hud.m_Next->setText("Next");
            break;
        }
    }
}};
struct ButtonNext_OnClick {void operator()(Button& button) const {
    HUD& hud = *static_cast<HUD*>(button.getUserPointer());
    switch (hud.m_GameState) {
        case GameState::Host_Server_Port_And_Name: {   
            hud.m_GameState = GameState::Host_Server_Map_And_Ship;

            hud.m_Core.startServer(55000);
            hud.m_Core.startClient(55000);

            hud.m_Next->setText("Enter Game");
            break;
        }case GameState::Host_Server_Map_And_Ship: {
            hud.m_GameState = GameState::Game;
            hud.m_Core.enterMap("Sol");
            hud.m_Next->setText("Next");
            break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            hud.m_GameState = GameState::Join_Server_Server_Info;
            hud.m_Next->setText("Enter Game");
            break;
        }case GameState::Join_Server_Server_Info: { 
            hud.m_GameState = GameState::Game;
            hud.m_Next->setText("Next");
            break;
        }default: {
            hud.m_GameState = GameState::Main_Menu;
            hud.m_Next->setText("Next");
            break;
        }
    }
}};


HUD::HUD(GameState::State& _state, Core& core):m_GameState(_state),m_Core(core){
    m_FontHandle = Resources::addFont(ResourceManifest::BasePath + "data/Fonts/consolas.fnt");
    m_Font = Resources::getFont(m_FontHandle);
    Engine::Math::setColor(m_Color, 255, 255, 0);
    m_Active = true;

    const auto& windowDimensions = Resources::getWindowSize();

    m_ButtonHost = new Button(*m_Font, glm::vec2(windowDimensions.x / 2, windowDimensions.y / 2), 150, 50);
    m_ButtonHost->setText("Host");
    m_ButtonHost->setColor(0.5f, 0.5f, 0.5f, 1);
    m_ButtonHost->setTextColor(1, 1, 0, 1);
    m_ButtonJoin = new Button(*m_Font, glm::vec2(windowDimensions.x / 2, (windowDimensions.y / 2) - 125), 150, 50);
    m_ButtonJoin->setText("Join");
    m_ButtonJoin->setColor(0.5f, 0.5f, 0.5f, 1);
    m_ButtonJoin->setTextColor(1, 1, 0, 1);
    
    m_ButtonHost->setUserPointer(this);
    m_ButtonJoin->setUserPointer(this);

    m_ButtonHost->setOnClickFunctor(ButtonHost_OnClick());
    m_ButtonJoin->setOnClickFunctor(ButtonJoin_OnClick());


    m_Back = new Button(*m_Font, glm::vec2(100, 50), 150, 50);
    m_Back->setText("Back");
    m_Back->setColor(0.5f, 0.5f, 0.5f, 1);
    m_Back->setTextColor(1, 1, 0, 1);
    m_Next = new Button(*m_Font, glm::vec2(windowDimensions.x - 100, 50), 150, 50);
    m_Next->setText("Next");
    m_Next->setColor(0.5f, 0.5f, 0.5f, 1);
    m_Next->setTextColor(1, 1, 0, 1);
    m_Back->setUserPointer(this);
    m_Next->setUserPointer(this);

    m_Back->setOnClickFunctor(ButtonBack_OnClick());
    m_Next->setOnClickFunctor(ButtonNext_OnClick());
}
HUD::~HUD() {
}


void HUD::onResize(const uint& width, const uint& height) {
    m_ButtonHost->setPosition(width / 2, height / 2);
    m_ButtonJoin->setPosition(width / 2, (height / 2) - 125);

    m_Back->setPosition(100, 50);
    m_Next->setPosition(width - 100, 50);
}

int _count = 0;
void HUD::update_game(const double& dt) {
    if (Engine::isKeyDownOnce(KeyboardKey::LeftAlt, KeyboardKey::X) || Engine::isKeyDownOnce(KeyboardKey::RightAlt, KeyboardKey::X)) {
        m_Active = !m_Active;
    }

    SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
    const auto& planets = scene->getPlanets();
    std::vector<Planet*> planetVector;
    planetVector.reserve(planets.size());

    for (auto& p : planets) {
        planetVector.push_back(p.second);
    }
    if (Engine::isKeyDownOnce(KeyboardKey::Comma)) {
        scene->getPlayer()->setTarget(planetVector[_count]->entity().getComponent<ComponentName>()->name());
        ++_count;
        if (_count > scene->getPlanets().size() - 1) {
            _count = 0;
        }
    }else if (Engine::isKeyDownOnce(KeyboardKey::Period)) {
        scene->getPlayer()->setTarget(planetVector[_count]->entity().getComponent<ComponentName>()->name());
        --_count;
        if (_count <= 0) {
            _count = scene->getPlanets().size() - 1;
        }
    }
}
void HUD::update_main_menu(const double& dt) {
    m_ButtonHost->update(dt);
    m_ButtonJoin->update(dt);
}
void HUD::update_host_server_map_and_ship(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);
}
void HUD::update_host_server_port_and_name(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);
}
void HUD::update_join_server_port_and_name_and_ip(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);
}
void HUD::update_join_server_server_info(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);
}

void HUD::render_game() {
    SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
    Ship* player = scene->getPlayer();
    glm::vec2 winSize = glm::vec2(Resources::getWindowSize().x, Resources::getWindowSize().y);

#pragma region renderCrossHairAndOtherInfo

    auto& target = player->getTarget();
    if (!target.null()) {
        auto& body = *target.getComponent<ComponentBody>();
        glm::vec3 pos = body.getScreenCoordinates(true);
        if (pos.z == 1) { //infront 
            auto boxPos = body.getScreenBoxCoordinates(8.0f);
            Material& crosshair = *(Material*)ResourceManifest::CrosshairMaterial.get();

            auto& crosshairTexture = *crosshair.getComponent(MaterialComponentType::Diffuse)->texture();
            const glm::vec4& color = glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1.0f);

            crosshairTexture.render(boxPos.topLeft, color, glm::radians(270.0f));
            crosshairTexture.render(boxPos.topRight, color, glm::radians(180.0f));
            crosshairTexture.render(boxPos.bottomLeft, color, 0.0f);
            crosshairTexture.render(boxPos.bottomRight, color, glm::radians(90.0f));

            auto& targetBody = *target.getComponent<ComponentBody>();
            auto& targetName = target.getComponent<ComponentName>()->name();

            unsigned long long distanceInKm = (targetBody.getDistanceLL(player->entity()) / 10);
            string stringRepresentation = "";
            if (distanceInKm > 0) {
                stringRepresentation = convertNumToNumWithCommas(unsigned long long(distanceInKm)) + " Km";
            }else {
                float distanceInm = (targetBody.getDistance(player->entity())) * 100.0f;
                stringRepresentation = to_string(uint(distanceInm)) + " m";
            }
            m_Font->renderText(targetName + "\n" + stringRepresentation, glm::vec2(pos.x + 40, pos.y - 15), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.7f, 0.7f), 0.1f);
        }
        else { //behind
            float angle = 0;
            Material& crosshairArrow = *(Material*)ResourceManifest::CrosshairArrowMaterial.get();
            auto& crosshairArrowTexture = *crosshairArrow.getComponent(MaterialComponentType::Diffuse)->texture();
            uint textureSizeOffset = (crosshairArrowTexture.width() / 2) + 4;
            if (pos.y > 2 && pos.y < winSize.y - 2) { //if y is within window bounds
                if (pos.x < 2) {
                    angle = -45;
                    pos.x += textureSizeOffset;
                }else {
                    angle = 135;
                    pos.x -= textureSizeOffset;
                }
            }else if (pos.y <= 1) { //if y is below the window bounds
                pos.y += textureSizeOffset;
                if (pos.x <= 1) { //bottom left corner
                    angle = 0;
                    pos.x += textureSizeOffset - 4;
                    pos.y -= 4;
                }else if (pos.x > winSize.x - 2) { //bottom right corner
                    angle = 90;
                    pos.x -= textureSizeOffset - 4;
                    pos.y -= 4;
                }else { //bottom normal
                    angle = 45;
                }
            }else { //if y is above the window bounds
                pos.y -= textureSizeOffset;
                if (pos.x < 2) { //top left corner
                    angle = -90;
                    pos.x += textureSizeOffset - 4;
                    pos.y += 4;
                }else if (pos.x > winSize.x - 2) { //top right corner
                    angle = 180;
                    pos.x -= textureSizeOffset - 4;
                    pos.y += 4;
                }else { //top normal
                    angle = -135;
                }
            }
            crosshairArrowTexture.render(glm::vec2(pos.x, pos.y), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1.0f), glm::radians(angle));
        }
    }

#pragma endregion

    if (!m_Active) return;

#pragma region DrawDebugStuff
    m_Font->renderText(Engine::Data::reportTime() +
        epriv::Core::m_Engine->m_DebugManager.reportDebug(),
        glm::vec2(10, Resources::getWindowSize().y - 10), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.8f, 0.8f), 0.1f);

#pragma endregion

}
void HUD::render_main_menu() {
    m_ButtonHost->render();
    m_ButtonJoin->render();
}
void HUD::render_host_server_map_and_ship() {
    m_Back->render();
    m_Next->render();
}
void HUD::render_host_server_port_and_name() {
    m_Back->render();
    m_Next->render();
}
void HUD::render_join_server_port_and_name_and_ip() {
    m_Back->render();
    m_Next->render();
}
void HUD::render_join_server_server_info() {
    m_Back->render();
    m_Next->render();
}

void HUD::update(const double& dt) {
    switch (m_GameState) {
        case GameState::Main_Menu: {
            update_main_menu(dt); break;
        }case GameState::Host_Server_Map_And_Ship: {
            update_host_server_map_and_ship(dt); break;
        }case GameState::Host_Server_Port_And_Name: {
            update_host_server_port_and_name(dt); break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            update_join_server_port_and_name_and_ip(dt); break;
        }case GameState::Join_Server_Server_Info: {
            update_join_server_server_info(dt); break;
        }case GameState::Game: {
            update_game(dt); break;
        }default: {
            break;
        }
    }
}
void HUD::render() {
    switch (m_GameState) {
        case GameState::Main_Menu: {
            render_main_menu(); break;
        }case GameState::Host_Server_Map_And_Ship: {
            render_host_server_map_and_ship(); break;
        }case GameState::Host_Server_Port_And_Name: {
            render_host_server_port_and_name(); break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            render_join_server_port_and_name_and_ip(); break;
        }case GameState::Join_Server_Server_Info: {
            render_join_server_server_info(); break;
        }case GameState::Game: {
            render_game(); break;
        }default: {
            break;
        }
    }
}

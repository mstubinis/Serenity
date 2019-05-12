#include "HUD.h"
#include "SolarSystem.h"
#include "Planet.h"
#include "Ship.h"
#include "ResourceManifest.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/Engine_Math.h>
#include <core/Font.h>
#include <core/engine/textures/Texture.h>
#include <core/Scene.h>
#include <core/Camera.h>
#include <core/Material.h>

#include <glm/vec4.hpp>

using namespace Engine;
using namespace std;

HUD::HUD() {
    m_Font = Resources::addFont(ResourceManifest::BasePath + "data/Fonts/consolas.fnt");
    Engine::Math::setColor(m_Color, 255, 255, 0);
    m_Active = true;
}
HUD::~HUD() {
}

uint _count = 0;
void HUD::update(const float& dt) {
    if (Engine::isKeyDownOnce(KeyboardKey::LeftAlt, KeyboardKey::Z)) {
        m_Active = !m_Active;
    }
    SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
    if (Engine::isKeyDownOnce(KeyboardKey::Comma)) {
        const unordered_map<string, Planet*>& planets = scene->getPlanets();
        uint a = 0;
        for (auto& p : planets) {
            if (a == _count) {
                scene->getPlayer()->setTarget(p.second->entity().getComponent<ComponentName>()->name());
                break;
            }
            ++a;
        }
        ++_count;
        if (_count > scene->getPlanets().size() - 1) { _count = 0; }
    }else if (Engine::isKeyDownOnce(KeyboardKey::Period)) {
        const unordered_map<string, Planet*>& planets = scene->getPlanets();
        uint a = 0;
        for (auto& p : planets) {
            if (a == _count) {
                scene->getPlayer()->setTarget(p.second->entity().getComponent<ComponentName>()->name());
                break;
            }
            ++a;
        }
        --_count;
        if (_count <= 0) { _count = scene->getPlanets().size() - 1; }
    }
}
void HUD::render() {
    if (!m_Active) return;

    //render hud stuff
    SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
    Ship* player = scene->getPlayer();
    glm::vec2 winSize = glm::vec2(Resources::getWindowSize().x, Resources::getWindowSize().y);

    Font* font = Resources::getFont(m_Font);

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
                float distanceInm = (targetBody.getDistance(player->entity()))*100.0f;
                stringRepresentation = to_string(uint(distanceInm)) + " m";
            }
            font->renderText(targetName + "\n" + stringRepresentation, glm::vec2(pos.x + 40, pos.y - 15), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.7f, 0.7f), 0.1f);
        }else{ //behind
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

#pragma region DrawDebugStuff
    font->renderText(Engine::Data::reportTime() +
        "\nWarping: " + (player->IsWarping() ? "True" : "False") +
        epriv::Core::m_Engine->m_DebugManager.reportDebug(),
        glm::vec2(10, Resources::getWindowSize().y - 10), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.8f, 0.8f), 0.1f);

#pragma endregion
}

#include "TargetRedicule.h"
#include "../map/Map.h"
#include "../Ship.h"
#include "../ResourceManifest.h"
#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace std;
using namespace Engine;

TargetRedicule::TargetRedicule(Map& map, Font& font):m_Map(map), m_Font(font){

}
TargetRedicule::~TargetRedicule() {

}

void TargetRedicule::update(const double& dt) {

}
void TargetRedicule::render() {
    auto* player = m_Map.getPlayer();
    const auto winSize = static_cast<glm::vec2>(Resources::getWindowSize());

#pragma region renderCrossHairAndOtherInfo

    auto target = player->getTarget();
    if (target) {
        auto& body = *target->getComponent<ComponentBody>();
        glm::vec3 pos = body.getScreenCoordinates(true);
        if (pos.z == 1) { //infront 
            auto boxPos = body.getScreenBoxCoordinates(8.0f);
            Material& crosshair = *(Material*)ResourceManifest::CrosshairMaterial.get();

            auto& crosshairTexture = *crosshair.getComponent(0).texture();
            const glm::vec4& color = glm::vec4(1, 1, 0, 1.0f);

            crosshairTexture.render(boxPos.topLeft, color, 270.0f);
            crosshairTexture.render(boxPos.topRight, color, 180.0f);
            crosshairTexture.render(boxPos.bottomLeft, color, 0.0f);
            crosshairTexture.render(boxPos.bottomRight, color, 90.0f);

            auto& targetBody = *target->getComponent<ComponentBody>();
            string name = "";
            auto targetName = target->getComponent<ComponentName>();
            if (targetName) {
                name = targetName->name();
            }
            unsigned long long distanceInKm = (targetBody.getDistanceLL(player->entity()) / 10);
            string stringRepresentation = "";
            if (distanceInKm > 0) {
                stringRepresentation = convertNumToNumWithCommas(unsigned long long(distanceInKm)) + " Km";
            }
            else {
                float distanceInm = (targetBody.getDistance(player->entity())) * 100.0f;
                stringRepresentation = to_string(uint(distanceInm)) + " m";
            }
            m_Font.renderText(name + "\n" + stringRepresentation, glm::vec2(pos.x + 20, pos.y + 20), glm::vec4(1, 1, 0, 1), 0, glm::vec2(0.7f, 0.7f), 0.1f);


            const auto healthDisplayWidthMax = 100.0f;
            Ship* ship_target = dynamic_cast<Ship*>(target);
            if (ship_target) {
                auto* shields = static_cast<ShipSystemShields*>(ship_target->getShipSystem(ShipSystemType::Shields));
                auto* _hull = static_cast<ShipSystemHull*>(ship_target->getShipSystem(ShipSystemType::Hull));
                if (shields) {
                    auto& shield = *shields;

                    auto startX = 0.0f;
                    auto incrX = (healthDisplayWidthMax / 6.0f) - 2.0f;
                    for (size_t i = 0; i < 6; ++i) {
                        Renderer::renderRectangle(glm::vec2((pos.x - (healthDisplayWidthMax / 2)) + startX, pos.y - 26.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), incrX, 2, 0, 0.10f, Alignment::BottomLeft);
                        Renderer::renderRectangle(glm::vec2((pos.x - (healthDisplayWidthMax / 2)) + startX, pos.y - 26.0f), glm::vec4(0.0f, 0.674f, 1.0f, 1.0f), shield.getHealthPercent(i) * incrX, 2, 0, 0.09f, Alignment::BottomLeft);
                        startX += (incrX + 3.0f);
                    }




                }
                if (_hull) {
                    auto& hull = *_hull;
                    Renderer::renderRectangle(glm::vec2(pos.x - (healthDisplayWidthMax / 2), pos.y - 27.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), healthDisplayWidthMax, 2, 0, 0.10f, Alignment::TopLeft);
                    Renderer::renderRectangle(glm::vec2(pos.x - (healthDisplayWidthMax / 2), pos.y - 27.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), hull.getHealthPercent() * healthDisplayWidthMax, 2, 0, 0.09f, Alignment::TopLeft);
                }
            }
        }
        else { //behind
            float angle = 0.0f;
            Material& crosshairArrow = *(Material*)ResourceManifest::CrosshairArrowMaterial.get();
            auto& crosshairArrowTexture = *crosshairArrow.getComponent(0).texture();
            uint textureSizeOffset = (crosshairArrowTexture.width() / 2) + 4;
            if (pos.y > 2 && pos.y < winSize.y - 2) { //if y is within window bounds
                if (pos.x < 2) {
                    angle = -45.0f;
                    pos.x += textureSizeOffset;
                }
                else {
                    angle = 135.0f;
                    pos.x -= textureSizeOffset;
                }
            }
            else if (pos.y <= 1) { //if y is below the window bounds
                pos.y += textureSizeOffset;
                if (pos.x <= 1) { //bottom left corner
                    angle = 0.0f;
                    pos.x += textureSizeOffset - 4;
                    pos.y -= 4;
                }
                else if (pos.x > winSize.x - 2) { //bottom right corner
                    angle = 90.0f;
                    pos.x -= textureSizeOffset - 4;
                    pos.y -= 4;
                }
                else { //bottom normal
                    angle = 45.0f;
                }
            }
            else { //if y is above the window bounds
                pos.y -= textureSizeOffset;
                if (pos.x < 2) { //top left corner
                    angle = -90.0f;
                    pos.x += textureSizeOffset - 4;
                    pos.y += 4;
                }
                else if (pos.x > winSize.x - 2) { //top right corner
                    angle = 180.0f;
                    pos.x -= textureSizeOffset - 4;
                    pos.y += 4;
                }
                else { //top normal
                    angle = -135.0f;
                }
            }
            crosshairArrowTexture.render(glm::vec2(pos.x, pos.y), glm::vec4(1, 1, 0, 1.0f), angle);
        }
    }
#pragma endregion
}
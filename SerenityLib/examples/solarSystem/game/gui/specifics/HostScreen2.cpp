#include "HostScreen2.h"

#include "MapDescriptionWindow.h"
#include "ServerLobbyChatWindow.h"
#include "../../Menu.h"
#include "../../Core.h"
#include "../Button.h"
#include "../TextBox.h"
#include "../Text.h"

#include "../../networking/client/Client.h"
#include "../../networking/server/Server.h"

#include "../../teams/TeamIncludes.h"
#include "../../teams/Team.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <regex>

using namespace Engine;
using namespace std;

const auto padding_x = 40.0f;
const auto padding_y = 40.0f;
const auto bottom_bar_height = 50.0f;
const auto bottom_bar_button_width = 150.0f;


HostScreen2::HostScreen2(Menu& menu, Font& font) : m_Menu(menu) {
    const auto winSize = Resources::getWindowSize();
    const auto contentSize = glm::vec2(winSize) - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;
}
HostScreen2::~HostScreen2() {

}
Menu& HostScreen2::getMenu() {
    return m_Menu;
}
void HostScreen2::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    const auto winSize = glm::uvec2(newWidth, newHeight);
    const auto contentSize = glm::vec2(winSize) - glm::vec2(padding_x * 2.0f, (padding_y * 2.0f) + bottom_bar_height);
    const auto top_content_height = contentSize.y / 2.0f;
    const auto first_2_boxes_width_top = contentSize.x - top_content_height;

}

void HostScreen2::update(const double& dt) {

}
void HostScreen2::render() {

}
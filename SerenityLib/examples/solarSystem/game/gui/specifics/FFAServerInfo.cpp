#include "FFAServerInfo.h"
#include "HostScreenFFA2.h"
#include "../../factions/Faction.h"
#include "../../map/Map.h"
#include "../Text.h"
#include "../ScrollFrame.h"
#include "../../ships/Ships.h"
#include "../../networking/server/Server.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace std;
using namespace Engine;

FFAServerInfo::FFAServerInfo(HostScreenFFA2& hostScreen, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font, x, y, width, height, depth, borderSize, labelText), m_HostScreen(hostScreen), m_Font(font){

    m_ScrollFrame = new ScrollFrame(font, x, y, width, height, depth);
}
FFAServerInfo::~FFAServerInfo() {
    SAFE_DELETE(m_ScrollFrame);
}
void FFAServerInfo::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    //m_ScrollFrame->setPosition(pos.x - (dividor_width / 2.0f) - scroll_frame_padding, pos.y - (frame_size.y / 2.0f) + 30.0f);
    //m_ScrollFrame->setSize(((frame_size.x - 60.0f) / 2.0f) - (scroll_frame_padding * 2.0f), frame_size.y - 110.0f);

}
void FFAServerInfo::update(const double& dt) {
    RoundedWindow::update(dt);

    m_ScrollFrame->update(dt);
}
void FFAServerInfo::render() {
    RoundedWindow::render();

    m_ScrollFrame->render();
}
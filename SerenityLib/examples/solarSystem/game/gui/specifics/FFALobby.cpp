#include "FFALobby.h"

#include "../../gui/Button.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace std;
using namespace Engine;


FFALobby::FFALobby(Font& font, const bool isHost) {
    m_IsHost           = isHost;
    m_ForwardButton    = nullptr;

    const auto& window_size = Resources::getWindowSize();

    if (isHost) {
        string start_text = "Start";
        const auto width = font.getTextWidth(start_text) + 10;
        const auto height = font.getTextHeight(start_text) + 10;
        glm::vec2 position = glm::vec2(window_size.x - width,0); //bottom right
        m_ForwardButton   = NEW Button(font, position, width, height);
        m_ForwardButton->setText(start_text);
    }
    string back_text = "Back";
    const auto width = font.getTextWidth(back_text) + 10;
    const auto height = font.getTextHeight(back_text) + 10;
    glm::vec2 position = glm::vec2(0,0); //bottom left
    m_BackButton     = NEW Button(font, position, width, height);
    m_BackButton->setText(back_text);
}
FFALobby::~FFALobby() {
    SAFE_DELETE(m_ForwardButton);
    SAFE_DELETE(m_BackButton);
}
void FFALobby::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    m_BackButton->setPosition(0, 0);
    if (m_ForwardButton)
        m_ForwardButton->setPosition(newWidth - m_ForwardButton->width(), 0);
}
void FFALobby::update(const double& dt) {
    //resize


    m_BackButton->update(dt);
    if (m_ForwardButton)
        m_ForwardButton->update(dt);
}
void FFALobby::render() {
    m_BackButton->render();
    if (m_ForwardButton)
        m_ForwardButton->render();
}
const bool FFALobby::isHost() const {
    return m_IsHost;
}
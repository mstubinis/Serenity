#include "MessageWithArrow.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>

#include "../Text.h"
#include "../../ResourceManifest.h"

#include <glm/glm.hpp>

using namespace std;
using namespace Engine;

MessageWithArrow::MessageWithArrow(Font& font, const string& text, const float& x, const float& y, const float& textScale) {
    m_Text = new Text(x, y, font, text);
    m_Text->setAlignment(Alignment::TopLeft);
    m_Text->setTextAlignment(TextAlignment::Left);
    m_Text->setColor(1.0f, 0.0f, 0.0f, 1.0f);
    m_Text->setTextScale(textScale, textScale);
    m_Shown = true;
    m_Timer = 0.0;
}
MessageWithArrow::~MessageWithArrow() {
    SAFE_DELETE(m_Text);
}

void MessageWithArrow::setPosition(const float& x, const float& y) {
    m_Text->setPosition(x, y);
}
void MessageWithArrow::setPosition(const glm::vec2& position) {
    MessageWithArrow::setPosition(position.x, position.y);
}

void MessageWithArrow::hide() {
    m_Shown = false;
}
void MessageWithArrow::show() {
    m_Shown = true;
}

void MessageWithArrow::update(const double& dt) {
    m_Text->update(dt);
    m_Timer += dt;
    if (m_Timer > (3.14159 * 2.0)) {
        m_Timer = 0.0;
    }
}
void MessageWithArrow::render() {
    if (!m_Shown)
        return;
    m_Text->render();

    Texture& arrow = *(Texture*)(ResourceManifest::GUIArrowBorder.get());

    const auto x_offset = glm::sin(m_Timer * 3.0) * 15.0;
    const auto pos = m_Text->positionWorld();

    Renderer::renderTexture(arrow, pos + glm::vec2(x_offset, -(m_Text->height() + 15.0f)), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(1.0f), 0.001f, Alignment::TopLeft);
}
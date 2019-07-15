#include "Button.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/events/Engine_Events.h>

using namespace Engine;

namespace Engine {
    namespace epriv {
        struct emptyFunctor { template<class T> void operator()(T& r) const {} };
    };
};

Button::Button(Font& font, const float& x, const float& y, const float& width, const float& height) :Widget(x, y, width, height) {
    setFont(font);
    m_Text = "Button";
    epriv::emptyFunctor empty;
    setOnClickFunctor(empty);
}
Button::Button(Font& font, const glm::vec2& position, const float& width, const float& height) : Widget(position, width, height) {
    setFont(font);
    m_Text = "Button";
    epriv::emptyFunctor empty;
    setOnClickFunctor(empty);
}
Button::~Button() {

}
const std::string& Button::text() const {
    return m_Text;
}
void Button::setFont(Font& font) {
    m_Font = &font;
}
void Button::setText(const char* text) {
    m_Text = text;
}
void Button::setText(const std::string& text) {
    m_Text = text;
}
void Button::update(const double& dt) {
    Widget::update(dt);
    if (m_MouseIsOver == true && Engine::isMouseButtonDownOnce(MouseButton::Left) == true) {
        m_FunctorOnClick();
    }
}
void Button::render() {
    float xOffset, yOffset;
    xOffset = yOffset = 0;
    switch (m_Alignment) {
        case WidgetAlignment::TopLeft: {
            xOffset -= m_Width / 2;
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::TopCenter: {
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::TopRight: {
            xOffset += m_Width / 2;
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::Left: {
            xOffset -= m_Width / 2;
            break;
        }case WidgetAlignment::Center: {
            break;
        }case WidgetAlignment::Right: {
            xOffset += m_Width / 2;
            break;
        }case WidgetAlignment::BottomLeft: {
            xOffset -= m_Width / 2;
            yOffset += m_Height / 2;
            break;
        }case WidgetAlignment::BottomCenter: {
            yOffset += m_Height / 2;
            break;
        }case WidgetAlignment::BottomRight: {
            xOffset += m_Width / 2;
            yOffset += m_Height / 2;
            break;
        }default: {
            break;
        }
    }

    glm::vec4 color = m_Color;
    if (isMouseOver()) {
        color *= 0.8f;
    }
    
    const auto newPos = glm::vec2(m_Position.x + xOffset, m_Position.y + yOffset);
    Renderer::renderRectangle(newPos, color * 0.3f, m_Width, m_Height, 0, 0.01f);

    const float lineHeight = m_Font->getTextHeight(m_Text);
    const auto newPosTxt = glm::vec2(m_Position.x, m_Position.y + lineHeight);
    m_Font->renderText(m_Text, newPosTxt, color,0,glm::vec2(1.0f), 0.008f,TextAlignment::Center);
}
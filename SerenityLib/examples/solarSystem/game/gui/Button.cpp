#include "Button.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/fonts/Font.h>

using namespace Engine;
using namespace std;

namespace Engine {
    namespace epriv {
        struct emptyFunctor { template<class T> void operator()(T* r) const {} };
    };
};

void Button::internalSetSize() {
    m_Height = getTextHeight() + m_Padding;
    m_Width = getTextWidth() + m_Padding;
}
Button::Button(const Font& font, const float x, const float y, const float width, const float height) : Widget(x, y, width, height) {
    setFont(font);
    setText("Button");
    setOnClickFunctor(epriv::emptyFunctor());
    setTextScale(1.0f, 1.0f);
    m_Padding = 20.0f;
    setAlignment(Alignment::Center);
    setTextAlignment(TextAlignment::Center);
    internalSetSize();
}
Button::Button(const Font& font, const glm::vec2& position, const float width, const float height) : Button(font, position.x, position.y, width, height) {
}
Button::~Button() {

}
void Button::setTextScale(const glm::vec2& scale) {
    m_TextScale = scale;
    internalSetSize();
}
void Button::setTextScale(const float x, const float y) {
    m_TextScale.x = x;
    m_TextScale.y = y;
    internalSetSize();
}
void Button::setTextAlignment(const TextAlignment::Type& textAlignment) {
    m_TextAlignment = textAlignment;
}
const TextAlignment::Type& Button::getTextAlignment() const {
    return m_TextAlignment;
}
const glm::vec2& Button::getTextScale() const {
    return m_TextScale;
}
const string& Button::text() const {
    return m_Text;
}

const float Button::getTextHeight() const {
    return (m_Font->getTextHeight(m_Text) * m_TextScale.y);
}
const float Button::getTextWidth() const {
    return (m_Font->getTextWidth(m_Text) * m_TextScale.x);
}

void Button::setTextColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_TextColor, r, g, b, a);
}
void Button::setTextColor(const glm::vec4& color) {
    Math::setColor(m_TextColor, color.r, color.g, color.b, color.a);
}
void Button::setFont(const Font& font) {
    m_Font = &const_cast<Font&>(font);
}
void Button::setText(const char* text) {
    m_Text = text;
    internalSetSize();
}
void Button::setText(const string& text) {
    m_Text = text;
    internalSetSize();
}
void Button::update(const double& dt) {
    Widget::update(dt);
    if (m_MouseIsOver == true && Engine::isMouseButtonDownOnce(MouseButton::Left) == true) {
        m_FunctorOnClick();
    }
}
void Button::render(const glm::vec4& scissor) {
    glm::vec4 color = m_Color;
    if (m_MouseIsOver)
        color += glm::vec4(0.15f);
    Renderer::renderRectangle(m_Position, color, m_Width, m_Height, 0, 0.01f, m_Alignment, scissor);
    glm::vec2 newPosTxt;
    switch (m_TextAlignment) {
    case TextAlignment::Left: {
        newPosTxt = glm::vec2(m_Position.x - m_Width / 2.0f, m_Position.y + getTextHeight());
        break;
    }case TextAlignment::Center: {
        newPosTxt = glm::vec2(m_Position.x, m_Position.y + getTextHeight());
        break;
    }case TextAlignment::Right: {
        newPosTxt = glm::vec2(m_Position.x + m_Width / 2.0f, m_Position.y + getTextHeight());
        break;
    }default: {
        newPosTxt = glm::vec2(m_Position.x, m_Position.y + getTextHeight());
        break;
    }
    }
    float fX = 0.0f;
    float fY = 0.0f;
    Renderer::alignmentOffset(m_Alignment, fX, fY, m_Width, m_Height);

    m_Font->renderText(m_Text, glm::vec2(newPosTxt.x + fX, newPosTxt.y + fY), m_TextColor, 0, m_TextScale, 0.008f, m_TextAlignment, scissor);
}
void Button::render() {
    glm::vec4 color = m_Color;
    if (m_MouseIsOver)
        color += glm::vec4(0.15f);
    Renderer::renderRectangle(m_Position, color, m_Width, m_Height, 0, 0.01f, m_Alignment);
    glm::vec2 newPosTxt; 
    switch (m_TextAlignment) {
        case TextAlignment::Left: {
            newPosTxt = glm::vec2(m_Position.x - m_Width / 2.0f, m_Position.y + getTextHeight());
            break;
        }case TextAlignment::Center: {
            newPosTxt = glm::vec2(m_Position.x, m_Position.y + getTextHeight());
            break;
        }case TextAlignment::Right: {
            newPosTxt = glm::vec2(m_Position.x + m_Width / 2.0f, m_Position.y + getTextHeight());
            break;
        }default: {
            newPosTxt = glm::vec2(m_Position.x, m_Position.y + getTextHeight());
            break;
        }
    }
    float fX = 0.0f;
    float fY = 0.0f;
    Renderer::alignmentOffset(m_Alignment, fX, fY, m_Width, m_Height);

    m_Font->renderText(m_Text, glm::vec2(newPosTxt.x + fX, newPosTxt.y + fY), m_TextColor, 0, m_TextScale, 0.008f, m_TextAlignment);
}
#include "Text.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace Engine;
using namespace std;

Text::Text(const float& x, const float& y, Font& font, const string& text):Widget(x,y,1,1) {
    m_Font          = &font;
    m_TextScale     = glm::vec2(1.0f);
    m_Text          = text;
    m_TextAlignment = TextAlignment::Left;

    internalUpdateSize();
}
Text::~Text() {

}
void Text::internalUpdateSize() {
    m_Width = width();
    m_Height = height();
}
void Text::setTextAlignment(const TextAlignment::Type& alignType) {
    m_TextAlignment = alignType;
}
void Text::setFont(const Font& font) {
    m_Font = &const_cast<Font&>(font);
    internalUpdateSize();
}
void Text::setText(const string& text) {
    m_Text = text;
    internalUpdateSize();
}
void Text::setTextScale(const float& x, const float& y) {
    m_TextScale.x = x;
    m_TextScale.y = y;
    internalUpdateSize();
}
void Text::setTextScale(const glm::vec2& scale) {
    m_TextScale = scale;
    internalUpdateSize();
}

const float Text::width() const {
    return m_Font->getTextWidth(m_Text) * m_TextScale.x;
}
const float Text::height() const {
    return (m_Font->getTextHeight(m_Text) * m_TextScale.y * 1.32f); //yes, this is evil
}
const string& Text::text() const {
    return m_Text;
}
const Font& Text::font() const {
    return *m_Font;
}
const glm::vec2& Text::textScale() const {
    return m_TextScale;
}
void Text::update(const float& dt) {

}
void Text::render() {
    Renderer::renderText(m_Text, *m_Font, m_Position, m_Color, 0, m_TextScale, 0.004f, m_TextAlignment);
}
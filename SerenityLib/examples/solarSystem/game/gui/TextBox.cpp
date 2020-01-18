#include "TextBox.h"
#include "Text.h"
#include <core/engine/fonts/Font.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace std;
using namespace Engine;


struct OnClick final {void operator()(TextBox* textBox) const {
    textBox->m_Active = true;
    textBox->m_Timer = 0.0f;
}};
struct OnEnter final {void operator()(TextBox* textBox) const {
}};



TextBox::TextBox(const string& label, const Font& font, const unsigned short maxCharacters, const float x, const float y) : Button(font, x, y, 25.0f, 25.0f) {
    m_MaxCharacters = maxCharacters;
    m_TextAlignment = TextAlignment::Left;
    m_Active = false;
    unlock();
    m_Timer = 0.0f;
    m_LabelTextColor = glm::vec4(1.0f);
    setText("");
    setLabel(label);

    internalUpdateSize();

    setOnClickFunctor(OnClick());
    setOnEnterFunctor(OnEnter());
    registerEvent(EventType::TextEntered);

    setTextureCorner(nullptr);
    setTextureEdge(nullptr);
    setTextureCornerHighlight(nullptr);
    setTextureEdgeHighlight(nullptr);
    enableTextureEdge(false);
    enableTextureCorner(false);

    TextBox::setPosition(x, y);
}
TextBox::TextBox(const string& label, const Font& font, const unsigned short maxCharacters, const glm::vec2& position) : TextBox(label, font, maxCharacters, position.x, position.y) {

}

TextBox::~TextBox() {
    unregisterEvent(EventType::TextEntered);
}
void TextBox::lock() {
    m_Locked = true;
    disable();
    disableMouseover();
}
void TextBox::unlock() {
    m_Locked = false;
    enable();
    enableMouseover();
}
const bool TextBox::isLocked() const{
    return m_Locked;
}
void TextBox::setLabelTextColor(const float r, const float g, const float b, const float a) {
    m_LabelTextColor = glm::vec4(r, g, b, a);
}
void TextBox::setInputTextColor(const float r, const float g, const float b, const float a) {
    m_TextColor = glm::vec4(r, g, b, a);
}
void TextBox::setLabelTextColor(const glm::vec4& color) {
    TextBox::setLabelTextColor(color.r, color.g, color.b, color.a);
}
void TextBox::setInputTextColor(const glm::vec4& color) {
    TextBox::setInputTextColor(color.r, color.g, color.b, color.a);
}
void TextBox::internalUpdateSize() {
    const auto input_width = (((m_Font->getTextWidth("X") * m_MaxCharacters) + 20.0f) * m_TextScale.x);
    m_Width = input_width;
    m_Height = (m_Font->getMaxHeight()) * m_TextScale.y;
}
void TextBox::setWidth(const float width) {
    const auto label_width = ((m_Font->getTextWidth(m_Label) * m_TextScale.x) );
    Widget::setWidth(glm::abs(width - label_width));
}
void TextBox::setHeight(const float height) {
    Widget::setHeight(height);
}
void TextBox::setSize(const float width, const float height) {
    TextBox::setWidth(width);
    TextBox::setHeight(height);
}
const float TextBox::width() const {
    const auto label_width = ((m_Font->getTextWidth(m_Label) * m_TextScale.x) );
    return m_Width + label_width;
}
const float TextBox::height() const {
    return Widget::height();
}
const glm::vec2 TextBox::positionLocal() const {
    const auto label_width = ((m_Font->getTextWidth(m_Label) * m_TextScale.x) / 2.0f);
    return Widget::positionLocal() + glm::vec2(label_width, 0.0f);
}
const glm::vec2 TextBox::positionWorld() const {
    const auto label_width = ((m_Font->getTextWidth(m_Label) * m_TextScale.x) / 2.0f);
    return Widget::positionWorld() + glm::vec2(label_width, 0.0f);
}
const glm::vec2 TextBox::position(const bool local) const {
    const auto label_width = ((m_Font->getTextWidth(m_Label) * m_TextScale.x) / 2.0f);
    return Widget::position(local) + glm::vec2(label_width, 0.0f);
}
void TextBox::setPosition(const float x, const float y) {
    Widget::setPosition(x, y);
}
void TextBox::setPosition(const glm::vec2& position) {
    Widget::setPosition(position);
}
const string& TextBox::getLabel() const {
    return m_Label;
}
void TextBox::setLabel(const char* label) {
    m_Label = label;
    internalUpdateSize();
}
void TextBox::setLabel(const string& label) {
    TextBox::setLabel(label.c_str());
}
void TextBox::setTextScale(const float x, const float y) {
    m_TextScale.x = x;
    m_TextScale.y = y;
    internalUpdateSize();
}
void TextBox::setTextScale(const glm::vec2& scale) {
    m_TextScale = scale;
    internalUpdateSize();
}
void TextBox::setText(const string& text) {
    if (text.size() > m_MaxCharacters)
        return;
    m_Text = text;
}
void TextBox::onEvent(const Event& e) {
    if (m_Active && !m_Locked) {
        if (e.type == EventType::TextEntered) {
            auto input = e.eventTextEntered.convert();
            auto unicode = e.eventTextEntered.unicode;
            if (unicode == 8) { //backspace
                if (m_Text.size() > 0) {
                    m_Text.pop_back();
                }
            }else{
                if (m_Text.size() < m_MaxCharacters) {
                    if (!input.empty()) {
                        if (m_Text.size() == 0) {
                            m_Text += std::toupper(input[0]);
                        }else{
                            m_Text += input;
                        }
                    }
                }
            }
        }
    }
}
void TextBox::update(const double& dt) {
    Button::update(dt);
    if ((m_Active && !m_MouseIsOver && Engine::isMouseButtonDownOnce(MouseButton::Left) == true)) {
        m_Active = false;
        m_Timer = 0.0f;
    }
    if (m_Active && !m_Locked) {
        m_Timer += static_cast<float>(dt);
        if (m_Timer >= 1.0f) {
            m_Timer = 0.0f;
        }
        if (Engine::isKeyDownOnce()) {
            const auto& key = Engine::getPressedKey();
            if (key == KeyboardKey::Return) {
                m_OnEnter();
                m_Active = false;
                m_Timer = 0.0f;
            }
        }
    }
}
void TextBox::render(const glm::vec4& scissor) {
    Button::render(scissor);

    const auto lineHeight = m_Font->getMaxHeight() * m_TextScale.y;

    const auto posW = positionFromAlignmentWorld();
    const auto pos = glm::vec2(posW.x, posW.y + (lineHeight));

    m_Font->renderText(m_Label, pos, m_LabelTextColor, 0, glm::vec2(m_TextScale), getDepth() - 0.001f, TextAlignment::Right, scissor);

    if (m_Active && m_Timer <= 0.5f && !m_Locked) {
        const auto blinkerPos = glm::vec2(posW.x + getTextWidth() + 4.0f + getPaddingSize(0), posW.y + height() / 2.0f);
        Renderer::renderRectangle(blinkerPos, glm::vec4(1.0f), 3, lineHeight - 4, 0, getDepth() - 0.002f, Alignment::Left, scissor);
    }
    if (m_Locked) {
        Renderer::renderRectangle(posW, m_TextColor * glm::vec4(1.0f, 1.0f, 1.0f, 0.25f), m_Width,m_Height, 0, getDepth() - 0.0021f, Alignment::BottomLeft, scissor);
    }
}
void TextBox::render() {
    TextBox::render(glm::vec4(-1.0f));
}
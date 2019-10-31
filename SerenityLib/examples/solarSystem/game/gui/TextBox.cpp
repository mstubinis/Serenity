#include "TextBox.h"
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



TextBox::TextBox(const string& label, const Font& font, const unsigned short maxCharacters, const float x, const float y):Button(font, x, y, 25.0f, 25.0f) {
    m_Text          = "";
    m_Active        = false;
    m_Timer         = 0.0f;
    m_Label         = label;
    m_MaxCharacters = maxCharacters;

    internalUpdateSize();

    setOnClickFunctor(OnClick());
    setOnEnterFunctor(OnEnter());
    m_TextAlignment = TextAlignment::Left;
    registerEvent(EventType::TextEntered);
}
TextBox::TextBox(const string& label, const Font& font, const unsigned short maxCharacters, const glm::vec2& position) : TextBox(label, font, maxCharacters, position.x, position.y) {

}

TextBox::~TextBox() {
    unregisterEvent(EventType::TextEntered);
}

void TextBox::internalUpdateSize() {
    m_Width = ((m_Font->getTextWidth("X") * m_MaxCharacters) + 20.0f) * m_TextScale.x;
    m_Height = (m_Font->getTextHeight("X") + 20.0f) * m_TextScale.y;
}

const string& TextBox::getLabel() const {
    return m_Label;
}
void TextBox::setLabel(const char* label) {
    m_Label = label;
}
void TextBox::setLabel(const string& label) {
    m_Label = label;
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
    if (m_Active) {
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
    if (m_Active) {
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

    const auto finalLabelString = m_Label + ": ";
    const auto lineHeight = m_Font->getTextHeight(finalLabelString) * m_TextScale.y;

    const auto pos = glm::vec2(m_Position.x - (m_Width / 2.0f) - 4.0f, m_Position.y + lineHeight);

    m_Font->renderText(finalLabelString, pos, m_TextColor, 0, glm::vec2(m_TextScale), 0.008f, TextAlignment::Right, scissor);

    if (m_Active && m_Timer <= 0.5f) {
        const glm::vec2 blinkerPos = glm::vec2(m_Position.x - (m_Width / 2.0f) + getTextWidth() + 4.0f, m_Position.y);
        Renderer::renderRectangle(blinkerPos, glm::vec4(1.0f), 3, lineHeight + 8, 0, 0.004f, Alignment::Center, scissor);
    }
}
void TextBox::render() {
    Button::render();

    const auto finalLabelString = m_Label + ": ";
    const auto lineHeight = m_Font->getTextHeight(finalLabelString) * m_TextScale.y;

    const auto pos = glm::vec2(m_Position.x - (m_Width / 2.0f) - 4.0f, m_Position.y + lineHeight);

    m_Font->renderText(finalLabelString, pos, m_TextColor, 0, glm::vec2(m_TextScale), 0.008f, TextAlignment::Right);

    if (m_Active && m_Timer <= 0.5f) {
        const glm::vec2 blinkerPos = glm::vec2(m_Position.x - (m_Width / 2.0f) + getTextWidth() + 4.0f, m_Position.y);
        Renderer::renderRectangle(blinkerPos, glm::vec4(1.0f), 3, lineHeight + 8, 0, 0.004f);
    }
}
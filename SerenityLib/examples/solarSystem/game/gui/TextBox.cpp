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


TextBox::TextBox(const string& label, Font& font, const unsigned short& maxCharacters, const float& x, const float& y):Button(font,x,y,25,25) {
    m_Text = "";
    m_Active = false;
    m_Timer = 0.0f;
    m_Label = label;
    m_MaxCharacters = maxCharacters;

    m_Width = (font.getTextWidth("X") * maxCharacters) + 20;
    m_Height = font.getTextHeight("X") + 20;

    setOnClickFunctor(OnClick());

    m_TextAlignment = TextAlignment::Left;
}
TextBox::TextBox(const string& label, Font& font, const unsigned short& maxCharacters, const glm::vec2& position) : Button(font,position.x, position.y, 25, 25) {
    m_Text = "";
    m_Active = false;
    m_Timer = 0.0f;
    m_Label = label;
    m_MaxCharacters = maxCharacters;

    m_Width = (font.getTextWidth("X") * maxCharacters) + 20;
    m_Height = font.getTextHeight("X") + 20;

    setOnClickFunctor(OnClick());

    m_TextAlignment = TextAlignment::Left;
}
TextBox::~TextBox() {

}

const string& TextBox::getLabel() const {
    return m_Label;
}
void TextBox::setLabel(const char* label) {
    m_Label = label;
}
void TextBox::setLabel(const std::string& label) {
    m_Label = label;
}
void TextBox::setTextScale(const float& x, const float& y) {
    m_TextScale.x = x;
    m_TextScale.y = y;
}
void TextBox::setTextScale(const glm::vec2& scale) {
    m_TextScale = scale;
}
void TextBox::setText(const char* text) {
    if (sizeof(text) / sizeof(text[0]) > m_MaxCharacters)
        return;
    m_Text = text;
}
void TextBox::setText(const string& text) {
    if (text.size() > m_MaxCharacters)
        return;
    m_Text = text;
}

void TextBox::update(const double& dt) {
    Button::update(dt);
    if ((m_Active && !m_MouseIsOver && Engine::isMouseButtonDownOnce(MouseButton::Left) == true) || (m_Active && Engine::isKeyDownOnce(KeyboardKey::Return))) {
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
            if (key == KeyboardKey::BackSpace) {
                if (m_Text.size() > 0) {
                    m_Text = m_Text.substr(0, m_Text.size() - 1);
                }
            }else if(key != KeyboardKey::Return){
                if (m_Text.size() < m_MaxCharacters) {
                    if (m_Text.size() == 0) {
                        m_Text += Engine::mapKey(key);
                    }else{
                        m_Text += std::tolower(Engine::mapKey(key)[0]);
                    }
                }
            }
        }
    }
}
void TextBox::render() {
    Button::render();

    const auto finalLabelString = m_Label + ": ";
    const auto lineWidth = m_Font->getTextWidth(finalLabelString) * m_TextScale.x;
    const auto lineHeight = m_Font->getTextHeight(finalLabelString) * m_TextScale.y;
    const auto& pos = glm::vec2(m_Position.x - (m_Width/2) - lineWidth - 4, m_Position.y + lineHeight);

    m_Font->renderText(finalLabelString, pos, m_TextColor, 0, glm::vec2(m_TextScale), 0.008f);

    if (m_Active && m_Timer <= 0.5f) {
        const glm::vec2 blinkerPos = glm::vec2(m_Position.x - (m_Width / 2) + getTextWidth() + 7, m_Position.y);
        Renderer::renderRectangle(blinkerPos, glm::vec4(1.0f), 3, lineHeight + 8, 0, 0.004f);
    }
}
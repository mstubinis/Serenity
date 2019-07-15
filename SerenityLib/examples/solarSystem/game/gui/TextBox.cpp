#include "TextBox.h"
#include <core/engine/fonts/Font.h>
#include <core/engine/events/Engine_Events.h>


namespace Engine {
    namespace epriv {
        struct BasicFunctor { template<class T> void operator()(T& textBox) const {

        } };
    };
};


struct OnClick final {void operator()(TextBox & button) const {
    button.m_Active = true;
}};



TextBox::TextBox(Font& font, const unsigned short& maxCharacters, const float& x, const float& y, const float& width, const float& height):Button(font,x,y,width,height) {
    m_Text = "";
    m_Active = false;
    m_MaxCharacters = maxCharacters;
    setOnClickFunctor(OnClick());
}
TextBox::TextBox(Font& font, const unsigned short& maxCharacters, const glm::vec2& position, const float& width, const float& height) : Button(font,position.x, position.y, width, height) {
    m_Text = "";
    m_Active = false;
    m_MaxCharacters = maxCharacters;
    setOnClickFunctor(OnClick());
}
TextBox::~TextBox() {

}


void TextBox::setText(const char* text) {
    if (sizeof(text) / sizeof(text[0]) > m_MaxCharacters)
        return;
    m_Text = text;
}
void TextBox::setText(const std::string& text) {
    if (text.size() > m_MaxCharacters)
        return;
    m_Text = text;
}

void TextBox::update(const double& dt) {
    Button::update(dt);
    if ((!m_MouseIsOver && Engine::isMouseButtonDownOnce(MouseButton::Left)) || (m_Active && Engine::isKeyDownOnce(KeyboardKey::Return))) {
        m_Active = false;
    }
}
void TextBox::render() {
    Button::render();
}
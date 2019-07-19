#include "ScrollWindow.h"
#include "ScrollBar.h"
#include "Text.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/fonts/Font.h>

using namespace Engine;
using namespace std;

ScrollFrame::ScrollFrame(const float& x, const float& y, const float& w, const float& h):Widget(x,y,w,h) {
    m_ScrollBar = new ScrollBar(x + w, y, 20, h);
    m_BorderSize = 1;
    m_ContentPadding = 10.0f;
    m_ContentHeight = 0.0f;
}
ScrollFrame::~ScrollFrame() {
    SAFE_DELETE(m_ScrollBar);
    SAFE_DELETE_VECTOR(m_Content);
}
void ScrollFrame::internalAddContent() {
    float height = 0.0f;
    for (auto& widget : m_Content) {
        height += widget->height();
        height += m_ContentPadding;
    }
    m_ContentHeight = height;
    const auto& percent = (m_Height) / m_ContentHeight;
    m_ScrollBar->setSliderSize(percent);
}
const float ScrollFrame::width() const {
    return m_Width + m_ScrollBar->width();
}
const float ScrollFrame::height() const {
    return Widget::height();
}
vector<Widget*>& ScrollFrame::content() {
    return m_Content;
}
const float ScrollFrame::contentHeight() const {
    return m_ContentHeight;
}
void ScrollFrame::setBorderSize(const float border) {
    m_BorderSize = border;
    m_ScrollBar->setBorderSize(border);
}

void ScrollFrame::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ScrollBar->setColor(r, g, b, a);
    Widget::setColor(r, g, b, a);

}
void ScrollFrame::addContent(Widget* widget) {

    if (m_Content.size() == 0) {
        widget->setPosition(m_Position.x + 4, m_Position.y - (m_ContentPadding / 2));
    }else{
        widget->setPosition(m_Position.x + 4, (m_Position.y - (m_ContentPadding / 2)) + (m_ContentHeight + m_ContentPadding));
    }

    //modify text if needed
    Text* textWidget = dynamic_cast<Text*>(widget);
    if (textWidget) {
        int textwidth = 0;
        int count = 0;
        auto text = textWidget->text();
        for (auto itr = text.begin(); itr != text.end(); ++itr) {
            auto character = (*itr);
            if (character != '\0' && character != '\n') {
                textwidth += (textWidget->font().getGlyphData(character).width) * textWidget->textScale().x;
                if (textwidth > m_Width - 110) {
                    itr = text.insert(itr, '\n');
                    textwidth = 0;
                }
            }
            ++count;
        }
        textWidget->setText(text);
    }
    m_Content.push_back(widget);
    internalAddContent();
}

void ScrollFrame::setColor(const glm::vec4& color) {
    m_ScrollBar->setColor(color);
    Widget::setColor(color);
}

void ScrollFrame::update(const double& dt) {
    Widget::update(dt);
    m_ScrollBar->update(dt);

    float height = 0.0f;
    float scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2)));
    for (auto& widget : m_Content) {
        widget->setPosition(m_Position.x + 4, ((m_Position.y - (m_ContentPadding / 2)) - height) - scrollOffset);
        height += widget->height();
        height += m_ContentPadding;
        widget->update(dt);
    }

    if (m_MouseIsOver || m_ScrollBar->isMouseOver()) {
        const auto& delta = Engine::getMouseWheelDelta();
        if (delta != 0.0) {
            m_ScrollBar->scroll(static_cast<float>(delta) * 0.3f);

        }
    }

}
void ScrollFrame::render() {
    m_ScrollBar->render();

    const float& halfWidth = m_Width / 2;
    const float& halfHeight = m_Height / 2;
    //const float& halfBorderSize = static_cast<float>(m_BorderSize) / 2.0f;

    Renderer::renderBorder(m_BorderSize, glm::vec2(m_Position.x + halfWidth + m_BorderSize, m_Position.y - halfHeight), m_Color, m_Width, m_Height, 0, 0.008f);

    Renderer::scissor(m_Position.x, m_Position.y - (m_Height - m_BorderSize), m_Width, m_Height - m_BorderSize - 1);

    //scroll bar area background
    float scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2)));
    Renderer::renderRectangle(glm::vec2(m_Position.x + halfWidth + m_BorderSize, m_Position.y  - scrollOffset - (m_ContentHeight / 2)), glm::vec4(0.3f), m_Width, m_ContentHeight, 0, 0.009f);
   
    for (auto& widget : m_Content) {
        widget->render();
    }
    Renderer::scissorDisable();
}
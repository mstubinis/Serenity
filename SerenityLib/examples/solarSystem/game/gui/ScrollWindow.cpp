#include "ScrollWindow.h"
#include "ScrollBar.h"
#include "Text.h"

#include <core/engine/renderer/Engine_Renderer.h>

using namespace Engine;
using namespace std;

ScrollFrame::ScrollFrame(const float& x, const float& y, const float& w, const float& h):Widget(x,y,w,h) {
    m_ScrollBar = new ScrollBar(x + w, y, 20, h);
    m_BorderSize = 1;
    m_ContentPadding = 0.0f;
    m_ContentHeight = 0.0f;
}
ScrollFrame::~ScrollFrame() {
    SAFE_DELETE(m_ScrollBar);
    for (auto c : m_Content)
        SAFE_DELETE(c.second);
}
void ScrollFrame::internalAddContent() {
    float height = 0.0f;
    int i = 0;
    for (auto& widget : m_Content) {
        height += widget.second->height();
        height += m_ContentPadding;
        ++i;
    }
    m_ContentHeight = height;
    const auto& percent = (m_Height) / m_ContentHeight;
    m_ScrollBar->setSliderSize(percent);
}
const float  ScrollFrame::contentHeight() const {
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
void ScrollFrame::addContent(const string& key, Widget* widget) {
    string keyCopy = key;
    int count = 0;
    while (m_Content.count(keyCopy)) {
        keyCopy = key + to_string(count);
        ++count;
    }
    if (m_Content.size() == 0) {
        widget->setPosition(m_Position.x + 4, m_Position.y);
    }else{
        widget->setPosition(m_Position.x + 4, m_Position.y + (m_ContentHeight + m_ContentPadding));
    }

    m_Content.emplace(keyCopy,widget);
    internalAddContent();
}
void ScrollFrame::removeContent(const string& key) {
    if (m_Content.count(key)) {
        m_Content.erase(key);
        internalAddContent();
    }
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
    int i = 0;
    for (auto& widget : m_Content) {
        if (i == 0) {
            widget.second->setPosition(m_Position.x + 4, m_Position.y - scrollOffset);
        }else{
            height += widget.second->height();
            height += m_ContentPadding;
            widget.second->setPosition(m_Position.x + 4, (m_Position.y - height) - scrollOffset);
        }
        widget.second->update(dt);
        ++i;
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
        widget.second->render();
    }
    Renderer::scissorDisable();
}
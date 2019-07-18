#include "ScrollBar.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine.h>
#include <core/engine/Engine_Debugging.h>

using namespace Engine;
using namespace std;

ScrollBar::ScrollBar(const float& x, const float& y, const float& w, const float& h, const ScrollBarType::Type& type) :Widget(x, y, w, h) {
    m_Type                           = type;
    m_Alignment                      = WidgetAlignment::TopLeft;
    m_CurrentlyDragging              = false;
    m_BorderSize                     = 1;

    m_ScrollBarCurrentContentPercent = 1.0f;
    m_ScrollBarCurrentPosition       = 0.0f;
    m_ScrollBarStartAnchor           = 0.0f;
    m_DragSnapshot                   = 0.0f;

}
ScrollBar::~ScrollBar() {

}
void ScrollBar::setBorderSize(const float border) {
    m_BorderSize = border;
}

void ScrollBar::setSliderSize(const float percent) {
    m_ScrollBarCurrentContentPercent = glm::clamp(percent,0.01f,1.0f);
}
const float ScrollBar::getSliderPosition() const {
    return m_ScrollBarCurrentPosition;
}
const float ScrollBar::getSliderHeight() const {
    return m_ScrollBarCurrentContentPercent * (m_Height - (m_Width * 2));
}

void ScrollBar::setType(const ScrollBarType::Type& type) {
    m_Type = type;
}

void ScrollBar::update(const double& dt) {
    Widget::update(dt);

    if (m_Hidden)
        return;

    const float& halfBorderSize = static_cast<float>(m_BorderSize) / 2.0f;
    float halfHeight = m_Height / 2;
    float scrollHeightMax = m_Height - (m_Width * 2);
    float scrollBarHeight = getSliderHeight();
    float absoluteStartPoint = m_Position.y - ((scrollBarHeight / 2) + m_Width);
    m_ScrollBarStartAnchor          = absoluteStartPoint + m_ScrollBarCurrentPosition;

    const float& scrollBarY = m_ScrollBarStartAnchor - scrollBarHeight / 2;
    const auto& mouse = Engine::getMousePosition();

    bool mouseOver = true;
    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < scrollBarY || mouse.y > scrollBarY + scrollBarHeight) {
        mouseOver = false;
    }
    if (mouseOver) {
        if (Engine::isMouseButtonDownOnce(MouseButton::Left)) {
            m_CurrentlyDragging = true;
            m_DragSnapshot = mouse.y;
        }
    }
    const float& bottomMark = scrollHeightMax - (scrollBarHeight);
    if (m_CurrentlyDragging && m_ScrollBarCurrentContentPercent < 1.0f) {
        if (Engine::isMouseButtonDown(MouseButton::Left)) {
            if (m_ScrollBarStartAnchor > absoluteStartPoint) {
            }
            else if (m_ScrollBarStartAnchor < absoluteStartPoint - bottomMark) {
            }
            else {
                m_ScrollBarCurrentPosition += (mouse.y - m_DragSnapshot);
            }
            m_DragSnapshot = mouse.y;
        }else{
            m_CurrentlyDragging = false;
            m_DragSnapshot = 0.0f;
        }
    }
    
    if (m_ScrollBarStartAnchor > absoluteStartPoint) {
        m_ScrollBarCurrentPosition = 0.0f;
        m_ScrollBarStartAnchor = absoluteStartPoint + m_ScrollBarCurrentPosition;
    }else if (m_ScrollBarStartAnchor < absoluteStartPoint - bottomMark) {
        m_ScrollBarCurrentPosition = -bottomMark;
        m_ScrollBarStartAnchor = absoluteStartPoint + m_ScrollBarCurrentPosition;
    }
}
void ScrollBar::render() {
    const float& halfWidth = m_Width / 2;
    const float& halfHeight = m_Height / 2;

    const float& scrollHeightMax = m_Height - (m_Width * 2);
    const float& scrollBarHeight = getSliderHeight();

    const float& gap = m_BorderSize + 4;
    const float& halfBorderSize = static_cast<float>(m_BorderSize) / 2.0f;

    //draw the actual scroll bar
    Renderer::renderRectangle(glm::vec2(m_Position.x + halfWidth, m_ScrollBarStartAnchor), glm::vec4(0.7f), m_Width, scrollBarHeight, 0, 0.009f);

    //scroll bar area background
    Renderer::renderRectangle(glm::vec2(m_Position.x + halfWidth, m_Position.y - halfHeight - halfBorderSize), glm::vec4(0.3f), m_Width, scrollHeightMax + 1, 0, 0.010f);

    //border
    Renderer::renderBorder(m_BorderSize, glm::vec2(m_Position.x + halfWidth, m_Position.y - halfHeight), m_Color, m_Width, m_Height, 0, 0.008f);

    //inner borders
    Renderer::renderRectangle(glm::vec2(m_Position.x + halfWidth, m_Position.y - m_Width + halfBorderSize), m_Color, m_Width, m_BorderSize, 0, 0.009f);
    Renderer::renderRectangle(glm::vec2(m_Position.x + halfWidth, m_Position.y - (m_Height + halfBorderSize) + m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f);

    //button triangles
    Renderer::renderTriangle(glm::vec2(m_Position.x + halfWidth, m_Position.y - halfWidth - halfBorderSize), m_Color, 180, halfWidth - gap, halfWidth - gap, 0.007f);
    Renderer::renderTriangle(glm::vec2(m_Position.x + halfWidth, m_Position.y - m_Height + halfWidth + halfBorderSize), m_Color, 0, halfWidth - gap, halfWidth - gap, 0.007f);

}
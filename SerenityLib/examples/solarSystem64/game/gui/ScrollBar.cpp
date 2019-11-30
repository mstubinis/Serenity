#include "ScrollBar.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine.h>
#include <core/engine/utils/Engine_Debugging.h>

using namespace Engine;
using namespace std;

ScrollBar::ScrollBar(const float x, const float y, const float w, const float h, const ScrollBarType::Type& type) : Widget(x, y, w, h) {
    m_Type                           = type;
    m_Alignment                      = Alignment::TopLeft;
    m_CurrentlyDragging              = false;
    m_BorderSize                     = 1.0f;
    m_ScrollBarColor                 = glm::vec4(0.6f);

    m_ScrollBarCurrentContentPercent = 1.0f;
    m_ScrollBarCurrentPosition       = 0.0f;
    m_DragSnapshot                   = 0.0f;

    internalUpdateScrollbarPosition();
}
ScrollBar::~ScrollBar() {

}
void ScrollBar::setPosition(const float x, const float y) {
    Widget::setPosition(x, y);
    internalUpdateScrollbarPosition();
}
void ScrollBar::setPosition(const glm::vec2& position) {
    Widget::setPosition(position.x, position.y);
    internalUpdateScrollbarPosition();
}

void ScrollBar::setBorderSize(const float border) {
    m_BorderSize = border;
}

void ScrollBar::setSliderSize(const float percent) {
    m_ScrollBarCurrentContentPercent = glm::clamp(percent,0.01f,1.0f);
    internalUpdateScrollbarPosition();
}
const float ScrollBar::getSliderPosition() const {
    return m_ScrollBarCurrentPosition;
}
const float ScrollBar::getSliderHeight() const {
    auto part2 = (m_Height - (m_Width * 2.0f));
    auto res = (m_ScrollBarCurrentContentPercent) * part2;
    return res;
}

void ScrollBar::setType(const ScrollBarType::Type& type) {
    m_Type = type;
}
void ScrollBar::scroll(const float amount) {
    if (m_ScrollBarCurrentContentPercent < 1.0f) {
        m_ScrollBarCurrentPosition += amount;
        
        const float& scrollHeightMax = m_Height - (m_Width * 2.0f);
        const float& scrollBarHeight = getSliderHeight();
        const float& absoluteStartPoint = m_Position.y - ((scrollBarHeight / 2.0f) + m_Width);
        const float& bottomMark = scrollHeightMax - (scrollBarHeight);

        internalUpdateScrollbarPosition();

        if (m_ScrollBarStartAnchor > absoluteStartPoint) {
            m_ScrollBarCurrentPosition = 0.0f;
            m_ScrollBarStartAnchor = absoluteStartPoint;
        }else if (m_ScrollBarStartAnchor < absoluteStartPoint - bottomMark) {
            m_ScrollBarCurrentPosition = -bottomMark;
            m_ScrollBarStartAnchor = absoluteStartPoint + -bottomMark;
        }
    }
}
void ScrollBar::setScrollBarColor(const glm::vec4& color) {
    m_ScrollBarColor = color;
}
void ScrollBar::internalUpdateScrollbarPosition() {
    const float& absoluteStartPoint = m_Position.y - ((getSliderHeight() / 2.0f) + m_Width);
    m_ScrollBarStartAnchor = absoluteStartPoint + m_ScrollBarCurrentPosition;
}
void ScrollBar::update(const double& dt) {
    Widget::update(dt);

    if (m_Hidden)
        return;
    float scrollBarHeight = getSliderHeight();
    const float& scrollBarY = m_ScrollBarStartAnchor - scrollBarHeight / 2.0f;
    const auto& mouse = Engine::getMousePosition();

    bool mouseOver = true;
    bool mouseOverTopTriangle = true;
    bool mouseOverBottomTriangle = true;
    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < scrollBarY || mouse.y > scrollBarY + scrollBarHeight) {
        mouseOver = false;
    }
    if (m_ScrollBarCurrentContentPercent < 1.0f) {
        if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < m_Position.y - m_Width || mouse.y > m_Position.y) {
            mouseOverTopTriangle = false;
        }
        if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < (m_Position.y - m_Height) || mouse.y > (m_Position.y - m_Height) + m_Width) {
            mouseOverBottomTriangle = false;
        }
        if (mouseOverTopTriangle) {
            if (Engine::isMouseButtonDown(MouseButton::Left)) {
                scroll(1.0f);
            }
        }
        if (mouseOverBottomTriangle) {
            if (Engine::isMouseButtonDown(MouseButton::Left)) {
                scroll(-1.0f);
            }
        }
    }
    if (mouseOver) {
        if (Engine::isMouseButtonDownOnce(MouseButton::Left)) {
            m_CurrentlyDragging = true;
            m_DragSnapshot = mouse.y;
        }
    }
    if (m_CurrentlyDragging && m_ScrollBarCurrentContentPercent < 1.0f) {
        if (Engine::isMouseButtonDown(MouseButton::Left)) {
            scroll(mouse.y - m_DragSnapshot);
            m_DragSnapshot = mouse.y;
        }else{
            m_CurrentlyDragging = false;
            m_DragSnapshot = 0.0f;
        }
    }
}
void ScrollBar::render(const glm::vec4& scissor) {
    const auto& mouse = Engine::getMousePosition();
    const float& halfWidth = m_Width / 2.0f;

    const float& scrollHeightMax = m_Height - (m_Width * 2.0f);
    const float& scrollBarHeight = getSliderHeight();

    const float& gap = m_BorderSize + 4.0f;
    const float& halfBorderSize = m_BorderSize / 2.0f;

    bool mouseOverTopTriangle = true;
    bool mouseOverBottomTriangle = true;
    bool mouseOver = true;
    const float& scrollBarY = m_ScrollBarStartAnchor - scrollBarHeight / 2.0f;

    //draw the actual scroll bar

    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < scrollBarY || mouse.y > scrollBarY + scrollBarHeight) {
        mouseOver = false;
    }
    glm::vec4 color = m_ScrollBarColor;
    if (mouseOver && m_ScrollBarCurrentContentPercent < 1.0f)
        color += glm::vec4(0.15f);
    Renderer::renderRectangle(glm::vec2(m_Position.x + 1.0f, m_ScrollBarStartAnchor), color, m_Width, scrollBarHeight, 0, 0.009f, Alignment::Left, scissor);

    //scroll bar area background
    Renderer::renderRectangle(glm::vec2(m_Position.x + 1.0f, m_Position.y - m_Width - halfBorderSize), glm::vec4(0.3f), m_Width, scrollHeightMax + 1, 0, 0.010f, m_Alignment, scissor);

    //border
    Renderer::renderBorder(m_BorderSize, glm::vec2(m_Position.x + 2.0f, m_Position.y), m_Color, m_Width, m_Height, 0, 0.008f, m_Alignment, scissor);

    //inner borders
    Renderer::renderRectangle(glm::vec2(m_Position.x + 2.0f, m_Position.y - m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f, Alignment::BottomLeft, scissor);
    Renderer::renderRectangle(glm::vec2(m_Position.x + 2.0f, m_Position.y - m_Height + m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f, Alignment::TopLeft, scissor);

    //button triangles
    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < m_Position.y - m_Width || mouse.y > m_Position.y) {
        mouseOverTopTriangle = false;
    }
    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < (m_Position.y - m_Height) || mouse.y >(m_Position.y - m_Height) + m_Width) {
        mouseOverBottomTriangle = false;
    }

    glm::vec4 colorTop = m_Color;
    if (mouseOverTopTriangle)
        colorTop += glm::vec4(0.55f);

    glm::vec4 colorBottom = m_Color;
    if (mouseOverBottomTriangle)
        colorBottom += glm::vec4(0.55f);

    const float& triSize = halfWidth - 4.0f;

    Renderer::renderTriangle(glm::vec2(m_Position.x + halfWidth + 2.0f, m_Position.y - halfWidth), colorTop, 180, triSize, triSize, 0.007f, Alignment::Center, scissor);
    Renderer::renderTriangle(glm::vec2(m_Position.x + halfWidth + 2.0f, m_Position.y - m_Height + halfWidth), colorBottom, 0, triSize, triSize, 0.007f, Alignment::Center, scissor);
}
void ScrollBar::render() {
    const auto& mouse = Engine::getMousePosition();
    const float& halfWidth = m_Width / 2.0f;

    const float& scrollHeightMax = m_Height - (m_Width * 2.0f);
    const float& scrollBarHeight = getSliderHeight();

    const float& gap = m_BorderSize + 4.0f;
    const float& halfBorderSize = m_BorderSize / 2.0f;

    bool mouseOverTopTriangle    = true;
    bool mouseOverBottomTriangle = true;
    bool mouseOver               = true;
    const float& scrollBarY = m_ScrollBarStartAnchor - scrollBarHeight / 2.0f;

    //draw the actual scroll bar

    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < scrollBarY || mouse.y > scrollBarY + scrollBarHeight) {
        mouseOver = false;
    }
    glm::vec4 color = m_ScrollBarColor;
    if (mouseOver && m_ScrollBarCurrentContentPercent < 1.0f)
        color += glm::vec4(0.15f);
    Renderer::renderRectangle(glm::vec2(m_Position.x + 1.0f, m_ScrollBarStartAnchor), color, m_Width, scrollBarHeight, 0, 0.009f, Alignment::Left);

    //scroll bar area background
    Renderer::renderRectangle(glm::vec2(m_Position.x + 1.0f, m_Position.y - m_Width - halfBorderSize), glm::vec4(0.3f), m_Width, scrollHeightMax + 1, 0, 0.010f, m_Alignment);

    //border
    Renderer::renderBorder(m_BorderSize, glm::vec2(m_Position.x + 2.0f, m_Position.y), m_Color, m_Width, m_Height, 0, 0.008f, m_Alignment);

    //inner borders
    Renderer::renderRectangle(glm::vec2(m_Position.x + 2.0f, m_Position.y - m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f, Alignment::BottomLeft);
    Renderer::renderRectangle(glm::vec2(m_Position.x + 2.0f, m_Position.y - m_Height + m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f, Alignment::TopLeft);

    //button triangles
    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < m_Position.y - m_Width || mouse.y > m_Position.y) {
        mouseOverTopTriangle = false;
    }
    if (mouse.x < m_Position.x || mouse.x > m_Position.x + m_Width || mouse.y < (m_Position.y - m_Height) || mouse.y >(m_Position.y - m_Height) + m_Width) {
        mouseOverBottomTriangle = false;
    }

    glm::vec4 colorTop = m_Color;
    if (mouseOverTopTriangle)
        colorTop += glm::vec4(0.55f);

    glm::vec4 colorBottom = m_Color;
    if (mouseOverBottomTriangle)
        colorBottom += glm::vec4(0.55f);

    const float& triSize = halfWidth - 4.0f;

    Renderer::renderTriangle(glm::vec2(m_Position.x + halfWidth + 2.0f, m_Position.y - halfWidth), colorTop, 180, triSize, triSize, 0.007f,Alignment::Center);
    Renderer::renderTriangle(glm::vec2(m_Position.x + halfWidth + 2.0f, m_Position.y - m_Height + halfWidth), colorBottom, 0, triSize, triSize, 0.007f, Alignment::Center);

}
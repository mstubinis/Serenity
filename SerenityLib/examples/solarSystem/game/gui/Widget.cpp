#include "Widget.h"

#include <core/engine/Engine_Math.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace Engine;

Widget::Widget(const glm::vec2& position, const float& width, const float& height) {
    m_Alignment = WidgetAlignment::Center;
    setPosition(position);
    m_Width = width;
    m_Height = height;
    m_MouseIsOver = false;
    m_UserPointer = nullptr;
}
Widget::Widget(const float& x, const float& y, const float& width, const float& height) {
    m_Alignment = WidgetAlignment::Center;
    setPosition(x,y);
    m_Width = width;
    m_Height = height;
    m_MouseIsOver = false;
    m_UserPointer = nullptr;
}
Widget::~Widget() {

}
void* Widget::getUserPointer() const {
    return m_UserPointer;
}
void Widget::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}

const float& Widget::width() const {
    return m_Width;
}
const float& Widget::height() const {
    return m_Height;
}

const bool Widget::isMouseOver() const {
    return m_MouseIsOver;
}

void Widget::setPosition(const float& x, const float& y) {
    m_Position.x = x;
    m_Position.y = y;
}
void Widget::setPosition(const glm::vec2& position) {
    m_Position = position;
}

void Widget::setColor(const float& r, const float& g, const float& b, const float& a) {
    Engine::Math::setColor(m_Color, r, g, b, a);
}
void Widget::setColor(const glm::vec4& color) {
    Engine::Math::setColor(m_Color, color.r, color.g, color.b, color.a);
}
void Widget::setAlignment(const WidgetAlignment::Type& alignment) {
    m_Alignment = alignment;
}
void Widget::show() {
    m_Hidden = true;
}
void Widget::hide() {
    m_Hidden = false;
}

void Widget::update(const double& dt) {
    float xOffset, yOffset;
    xOffset = yOffset = 0;
    switch (m_Alignment) {
        case WidgetAlignment::TopLeft: {
            break;
        }case WidgetAlignment::TopCenter: {
            xOffset -= m_Width / 2;
            break;
        }case WidgetAlignment::TopRight: {
            xOffset -= m_Width;
            break;
        }case WidgetAlignment::Left: {
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::Center: {
            xOffset -= m_Width / 2;
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::Right: {
            xOffset -= m_Width;
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::BottomLeft: {
            yOffset -= m_Height;
            break;
        }case WidgetAlignment::BottomCenter: {
            xOffset -= m_Width / 2;
            yOffset -= m_Height;
            break;
        }case WidgetAlignment::BottomRight: {
            xOffset -= m_Width;
            yOffset -= m_Height;
            break;
        }default: {
            break;
        }
    }
    const auto& mouse = Engine::getMousePosition();
    const float& x = m_Position.x + xOffset;
    const float& y = m_Position.y + yOffset;

    if (mouse.x < x || mouse.x > x + m_Width || mouse.y < y || mouse.y > y + m_Height)
        m_MouseIsOver = false;
    else
        m_MouseIsOver = true;
}
void Widget::render() {

}
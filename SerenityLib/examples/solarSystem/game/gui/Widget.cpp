#include "Widget.h"

#include <core/engine/math/Engine_Math.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace Engine;

Widget::Widget(const glm::vec2& position, const unsigned int& width, const unsigned int& height) {
    m_Alignment = Alignment::TopLeft;
    setPosition(position);
    m_Width = width;
    m_Height = height;
    m_MouseIsOver = false;
    m_UserPointer = nullptr;
}
Widget::Widget(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height) {
    m_Alignment = Alignment::TopLeft;
    setPosition(x,y);
    m_Width = width;
    m_Height = height;
    m_MouseIsOver = false;
    m_UserPointer = nullptr;
}
Widget::~Widget() {

}
void Widget::setWidth(const unsigned int& width) {
    m_Width = width;
}
void Widget::setHeight(const unsigned int& height) {
    m_Height = height;
}
void Widget::setSize(const unsigned int& width, const unsigned int& height) {
    setWidth(width);
    setHeight(height);
}
const glm::vec4& Widget::color() const {
    return m_Color;
}
const glm::uvec2& Widget::position() const {
    return m_Position;
}
void* Widget::getUserPointer() const {
    return m_UserPointer;
}
void Widget::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}

const unsigned int Widget::width() const {
    return m_Width;
}
const unsigned int Widget::height() const {
    return m_Height;
}

const bool Widget::isMouseOver() const {
    return m_MouseIsOver;
}

void Widget::setPosition(const unsigned int& x, const unsigned int& y) {
    m_Position.x = x;
    m_Position.y = y;
}
void Widget::setPosition(const glm::uvec2& position) {
    m_Position = position;
}

void Widget::setColor(const float& r, const float& g, const float& b, const float& a) {
    Engine::Math::setColor(m_Color, r, g, b, a);
}
void Widget::setColor(const glm::vec4& color) {
    Engine::Math::setColor(m_Color, color.r, color.g, color.b, color.a);
}
void Widget::setAlignment(const Alignment::Type& alignment) {
    m_Alignment = alignment;
}
void Widget::show() {
    m_Hidden = true;
}
void Widget::hide() {
    m_Hidden = false;
}

void Widget::update(const double& dt) {
    const auto& mouse = Engine::getMousePosition();

    auto pos = Widget::positionFromAlignment();
    if (mouse.x < pos.x || mouse.x > pos.x + m_Width || mouse.y < pos.y || mouse.y > pos.y + m_Height)
        m_MouseIsOver = false;
    else
        m_MouseIsOver = true;
}

const glm::uvec2 Widget::positionFromAlignment(const unsigned int& width, const unsigned int& height,const Alignment::Type& alignment) {
    unsigned int xOffset, yOffset;
    xOffset = yOffset = 0;
    switch (alignment) {
        case Alignment::TopLeft: {
            yOffset -= height;
            break;
        }case Alignment::TopCenter: {
            xOffset -= width / 2;
            yOffset -= height;
            break;
        }case Alignment::TopRight: {
            xOffset -= width;
            yOffset -= height;
            break;
        }case Alignment::Left: {
            xOffset -= width;
            yOffset -= height / 2;
            break;
        }case Alignment::Center: {
            xOffset -= width / 2;
            yOffset -= height / 2;
            break;
        }case Alignment::Right: {
            yOffset -= height / 2;
            break;
        }case Alignment::BottomLeft: {
            break;
        }case Alignment::BottomCenter: {
            xOffset -= width / 2;
            break;
        }case Alignment::BottomRight: {
            xOffset -= width;
            break;
        }default: {
            break;
        }
    }
    const unsigned int& x = m_Position.x + xOffset;
    const unsigned int& y = m_Position.y + yOffset;
    return glm::uvec2(x, y);
}

const glm::uvec2 Widget::positionFromAlignment() {
    return positionFromAlignment(m_Width, m_Height, m_Alignment);
}

void Widget::render() {
}
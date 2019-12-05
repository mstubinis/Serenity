#include "Widget.h"

#include <core/engine/math/Engine_Math.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace Engine;

Widget::Widget(const float x, const float y, const float width, const float height) {
    m_Alignment   = Alignment::TopLeft;
    setPosition(x, y);
    m_Width       = width;
    m_Height      = height;
    m_MouseIsOver = false;
    m_UserPointer = nullptr;
    m_Parent      = nullptr;
}
Widget::Widget(const glm::vec2& position, const glm::vec2& size) : Widget(position.x, position.y, size.x, size.y){

}
Widget::Widget(const glm::vec2& position, const float width, const float height) : Widget(position.x, position.y, width, height) {

}
Widget::Widget(const float x, const float y, const glm::vec2& size) : Widget(x, y, size.x, size.y) {

}
Widget::~Widget() {

}
const bool Widget::setParent(Widget* parent) {
    if (m_Parent) {
        return false;
    }
    return parent->addChild(this);
}
const bool Widget::clearParent() {
    if (!m_Parent) {
        return false;
    }
    return m_Parent->removeChild(this);
}
const bool Widget::addChild(Widget* child) {
    for (auto& ptr : m_Children) {
        if (ptr == child)
            return false;
    }
    m_Children.push_back(child);
    child->m_Parent = this;
    return true;
}
const bool Widget::removeChild(Widget* child) {
    for (size_t i = 0; i < m_Children.size(); ++i) {
        auto* ptr = m_Children[i];
        if (ptr == child) {
            m_Children.erase(m_Children.begin() + i);
            child->m_Parent = nullptr;
            return true;
        }
    }
    return false;
}
void Widget::setWidth(const float width) {
    m_Width = width;
}
void Widget::setHeight(const float height) {
    m_Height = height;
}
void Widget::setSize(const float width, const float height) {
    setWidth(width);
    setHeight(height);
}
const glm::vec4& Widget::color() const {
    return m_Color;
}
const glm::vec2& Widget::positionLocal() const {
    return m_Position;
}
const glm::vec2  Widget::positionWorld() const {
    std::vector<Widget*> list;
    Widget* parent = m_Parent;
    while (parent) {
        list.push_back(parent);
        parent = parent->m_Parent;
    }
    glm::vec2 worldPos = positionLocal();
    for (int i = list.size() - 1; i >= 0; --i) {
        worldPos += list[i]->positionLocal();
    }
    return worldPos;
}
const glm::vec2& Widget::position(const bool local) const {
    if(local)
        return positionLocal();
    return positionWorld();
}
void* Widget::getUserPointer() const {
    return m_UserPointer;
}
void Widget::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}

const float Widget::width() const {
    return m_Width;
}
const float Widget::height() const {
    return m_Height;
}

const bool Widget::isMouseOver() const {
    return m_MouseIsOver;
}

void Widget::setPosition(const float x, const float y) {
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

    auto pos = Widget::positionFromAlignmentWorld();
    if (mouse.x < pos.x || mouse.x > pos.x + m_Width || mouse.y < pos.y || mouse.y > pos.y + m_Height)
        m_MouseIsOver = false;
    else
        m_MouseIsOver = true;

    for (auto& child : m_Children)
        child->update(dt);
}
const glm::vec2 Widget::positionFromAlignmentWorld(const float width, const float height, const Alignment::Type& alignment) {
    glm::vec2 offset(0.0f);
    switch (alignment) {
        case Alignment::TopLeft: {
            offset.y -= height;
            break;
        }case Alignment::TopCenter: {
            offset.x -= width / 2;
            offset.y -= height;
            break;
        }case Alignment::TopRight: {
            offset.x -= width;
            offset.y -= height;
            break;
        }case Alignment::Left: {
            offset.x -= width;
            offset.y -= height / 2;
            break;
        }case Alignment::Center: {
            offset.x -= width / 2;
            offset.y -= height / 2;
            break;
        }case Alignment::Right: {
            offset.y -= height / 2;
            break;
        }case Alignment::BottomLeft: {
            break;
        }case Alignment::BottomCenter: {
            offset.x -= width / 2;
            break;
        }case Alignment::BottomRight: {
            offset.x -= width;
            break;
        }default: {
            break;
        }
    }
    return positionWorld() + offset;
}
const glm::vec2 Widget::positionFromAlignmentWorld() {
    return positionFromAlignmentWorld(m_Width, m_Height, m_Alignment);
}
const glm::vec2 Widget::positionFromAlignment(const float width, const float height,const Alignment::Type& alignment) {
    glm::vec2 offset(0.0f);
    switch (alignment) {
        case Alignment::TopLeft: {
            offset.y -= height;
            break;
        }case Alignment::TopCenter: {
            offset.x -= width / 2;
            offset.y -= height;
            break;
        }case Alignment::TopRight: {
            offset.x -= width;
            offset.y -= height;
            break;
        }case Alignment::Left: {
            offset.x -= width;
            offset.y -= height / 2;
            break;
        }case Alignment::Center: {
            offset.x -= width / 2;
            offset.y -= height / 2;
            break;
        }case Alignment::Right: {
            offset.y -= height / 2;
            break;
        }case Alignment::BottomLeft: {
            break;
        }case Alignment::BottomCenter: {
            offset.x -= width / 2;
            break;
        }case Alignment::BottomRight: {
            offset.x -= width;
            break;
        }default: {
            break;
        }
    }
    return positionLocal() + offset;
}
const glm::vec2 Widget::positionFromAlignment() {
    return positionFromAlignment(m_Width, m_Height, m_Alignment);
}

void Widget::render() {
    for (auto& child : m_Children)
        child->render();
}
void Widget::render(const glm::vec4& scissor) {
    for (auto& child : m_Children)
        child->render(scissor);
}
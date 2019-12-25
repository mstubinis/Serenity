#include "ScrollBar.h"
#include "Button.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/system/Engine.h>
#include <core/engine/utils/Engine_Debugging.h>

using namespace Engine;
using namespace std;

struct ScrollBar_TopButtonFunctor final { void operator()(Button* button) const {
    auto* bar = static_cast<ScrollBar*>(button->getUserPointer());
    bar->scroll(1.0f);
}};
struct ScrollBar_BottomButtonFunctor final { void operator()(Button* button) const {
    auto* bar = static_cast<ScrollBar*>(button->getUserPointer());
    bar->scroll(-1.0f);
}};

ScrollBar::ScrollBar(const Font& font, const float x, const float y, const float w, const float h, const ScrollBarType::Type& type) : Widget(x, y, w, h) {
    m_Type                           = type;
    m_Alignment                      = Alignment::TopLeft;
    m_CurrentlyDragging              = false;
    m_BorderSize                     = 1.0f;
    m_ScrollBarColor                 = glm::vec4(0.6f);

    m_ScrollBarCurrentContentPercent = 1.0f;
    m_ScrollBarCurrentPosition       = 0.0f;
    m_DragSnapshot                   = 0.0f;


    m_TopOrLeftButton     = NEW Button(font, glm::vec2(x, y), w, w);
    m_TopOrLeftButton->setText("^");
    m_TopOrLeftButton->setAlignment(Alignment::TopLeft);
    m_TopOrLeftButton->setColor(m_Color);
    m_TopOrLeftButton->setTextColor(0, 0, 0, 1);
    m_TopOrLeftButton->setOnClickToBePulsed(true);

    m_TopOrLeftButton->setTextureCorner(nullptr);
    m_TopOrLeftButton->setTextureEdge(nullptr);
    m_TopOrLeftButton->setTextureCornerHighlight(nullptr);
    m_TopOrLeftButton->setTextureEdgeHighlight(nullptr);
    m_TopOrLeftButton->enableTextureCorner(false);
    m_TopOrLeftButton->enableTextureEdge(false);

    m_TopOrLeftButton->setUserPointer(this);
    m_TopOrLeftButton->setOnClickFunctor(ScrollBar_TopButtonFunctor());

    m_BottomOrRightButton = NEW Button(font, glm::vec2(x, y - h), w, w);
    m_BottomOrRightButton->setText("v");
    m_BottomOrRightButton->setAlignment(Alignment::BottomLeft);
    m_BottomOrRightButton->setColor(m_Color);
    m_BottomOrRightButton->setTextColor(0, 0, 0, 1);
    m_BottomOrRightButton->setOnClickToBePulsed(true);

    m_BottomOrRightButton->setTextureCorner(nullptr);
    m_BottomOrRightButton->setTextureEdge(nullptr);
    m_BottomOrRightButton->setTextureCornerHighlight(nullptr);
    m_BottomOrRightButton->setTextureEdgeHighlight(nullptr);
    m_BottomOrRightButton->enableTextureCorner(false);
    m_BottomOrRightButton->enableTextureEdge(false);

    m_BottomOrRightButton->setUserPointer(this);
    m_BottomOrRightButton->setOnClickFunctor(ScrollBar_BottomButtonFunctor());

    internalUpdateScrollbarPosition();
}
ScrollBar::~ScrollBar() {
    SAFE_DELETE(m_TopOrLeftButton);
    SAFE_DELETE(m_BottomOrRightButton);
}

void ScrollBar::resetScrollOffset() {
    scroll(999999999999.0f);
}
void ScrollBar::setColor(const float& r, const float& g, const float& b, const float& a) {
    Widget::setColor(r, g, b, a);
    m_TopOrLeftButton->setColor(r, g, b, a);
    m_BottomOrRightButton->setColor(r, g, b, a);
}
void ScrollBar::setColor(const glm::vec4& color) {
    ScrollBar::setColor(color.r, color.g, color.b, color.a);
}
void ScrollBar::setWidth(const float width) {
    Widget::setWidth(width);
    m_TopOrLeftButton->setSize(m_Width, m_Width);
    m_BottomOrRightButton->setSize(m_Width, m_Width);
}
void ScrollBar::setHeight(const float height) {
    Widget::setHeight(height);
    m_TopOrLeftButton->setSize(m_Width, m_Width);
    m_BottomOrRightButton->setSize(m_Width, m_Width);
}
void ScrollBar::setSize(const float width, const float height) {
    ScrollBar::setWidth(width);
    ScrollBar::setHeight(height);
}
void ScrollBar::setPosition(const float x, const float y) {
    Widget::setPosition(x, y);

    m_TopOrLeftButton->setPosition(x, y);
    m_BottomOrRightButton->setPosition(x, y - height());

    internalUpdateScrollbarPosition();
}
void ScrollBar::setPosition(const glm::vec2& position) {
    ScrollBar::setPosition(position.x, position.y);
}

void ScrollBar::setBorderSize(const float border) {
    m_BorderSize = border;
}

void ScrollBar::setSliderSize(const float percent) {
    m_ScrollBarCurrentContentPercent = glm::clamp(percent,0.01f,1.0f);
    internalUpdateScrollbarPosition();
}
const bool ScrollBar::isScrollable() const {
    return (m_ScrollBarCurrentContentPercent < 1.0f) ? true : false;
}
const float ScrollBar::getSliderPosition() const {
    return m_ScrollBarCurrentPosition;
}
const float ScrollBar::getSliderHeight() const {
    auto part2 = (m_Height - (m_Width * 2.0f));
    auto res   = (m_ScrollBarCurrentContentPercent) * part2;
    return res;
}

void ScrollBar::setType(const ScrollBarType::Type& type) {
    m_Type = type;
}
void ScrollBar::scroll(const float amount) {
    if (m_ScrollBarCurrentContentPercent < 1.0f) {
        m_ScrollBarCurrentPosition += amount;
        
        const float& scrollHeightMax    = m_Height - (m_Width * 2.0f);
        const float& scrollBarHeight    = getSliderHeight();
        const float& absoluteStartPoint = m_Position.y - ((scrollBarHeight / 2.0f) + m_Width);
        const float& bottomMark         = scrollHeightMax - scrollBarHeight;

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
    m_TopOrLeftButton->update(dt);
    m_BottomOrRightButton->update(dt);

    if (m_Hidden)
        return;
    float scrollBarHeight   = getSliderHeight();
    const float& scrollBarY = m_ScrollBarStartAnchor - scrollBarHeight / 2.0f;
    const auto& mouse       = Engine::getMousePosition();

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
    const auto pos = positionWorld();

    if (mouse.x < pos.x || mouse.x > pos.x + m_Width || mouse.y < scrollBarY || mouse.y > scrollBarY + scrollBarHeight) {
        mouseOver = false;
    }
    glm::vec4 color = m_ScrollBarColor;
    if (mouseOver && m_ScrollBarCurrentContentPercent < 1.0f)
        color += glm::vec4(0.15f);
    Renderer::renderRectangle((glm::ivec2(pos.x , m_ScrollBarStartAnchor)), color, m_Width, scrollBarHeight, 0, 0.009f, Alignment::Left, scissor);

    //scroll bar area background
    Renderer::renderRectangle((glm::ivec2(pos.x , pos.y - m_Width - halfBorderSize)), glm::vec4(0.3f), m_Width, scrollHeightMax + 1, 0, 0.010f, m_Alignment, scissor);

    //border
    //Renderer::renderBorder(m_BorderSize, glm::vec2(pos.x + 2.0f, pos.y), m_Color, m_Width, m_Height, 0, 0.008f, m_Alignment, scissor);

    //inner borders
    //Renderer::renderRectangle(glm::vec2(pos.x + 2.0f, pos.y - m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f, Alignment::BottomLeft, scissor);
    //Renderer::renderRectangle(glm::vec2(pos.x + 2.0f, pos.y - m_Height + m_Width), m_Color, m_Width, m_BorderSize, 0, 0.009f, Alignment::TopLeft, scissor);

    //button triangles
    if (mouse.x < pos.x || mouse.x > pos.x + m_Width || mouse.y < pos.y - m_Width || mouse.y > pos.y) {
        mouseOverTopTriangle = false;
    }
    if (mouse.x < pos.x || mouse.x > pos.x + m_Width || mouse.y < (pos.y - m_Height) || mouse.y >(pos.y - m_Height) + m_Width) {
        mouseOverBottomTriangle = false;
    }

    glm::vec4 colorTop = m_Color;
    if (mouseOverTopTriangle)
        colorTop += glm::vec4(0.55f);

    glm::vec4 colorBottom = m_Color;
    if (mouseOverBottomTriangle)
        colorBottom += glm::vec4(0.55f);

    const float& triSize = halfWidth - 4.0f;

    //Renderer::renderTriangle(glm::vec2(pos.x + halfWidth + 2.0f, pos.y - halfWidth), colorTop, 180, triSize, triSize, 0.007f, Alignment::Center, scissor);
    //Renderer::renderTriangle(glm::vec2(pos.x + halfWidth + 2.0f, pos.y - m_Height + halfWidth), colorBottom, 0, triSize, triSize, 0.007f, Alignment::Center, scissor);

    m_TopOrLeftButton->render();
    m_BottomOrRightButton->render();

    Widget::render(scissor);
}
void ScrollBar::render() {
    ScrollBar::render(glm::vec4(-1.0f));
}
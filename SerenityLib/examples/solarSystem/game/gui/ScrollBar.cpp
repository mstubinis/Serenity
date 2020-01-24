#include "ScrollBar.h"
#include "Button.h"

#include "../factions/Faction.h"
#include "../ResourceManifest.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Renderer.h>
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
struct ScrollBar_Functor final { void operator()(Button* button) const {
    //this logic is handled in update() and render()
    //auto* bar = static_cast<ScrollBar*>(button->getUserPointer());
    //bar->scroll(-1.0f);
}};

const unsigned int default_padding = 7;

ScrollBar::ScrollBar(const Font& font, const float x, const float y, const float w, const float h, const float depth, const ScrollBarType::Type& type) : Widget(x, y, w, h){
    m_Type                           = type;
    m_Alignment                      = Alignment::TopLeft;
    m_CurrentlyDragging              = false;
    m_ScrollBarColor                 = glm::vec4(0.6f);

    m_ScrollBarCurrentContentPercent = 1.0f;
    m_ScrollBarCurrentPosition       = 0.0f;
    m_DragSnapshot                   = 0.0f;


    m_ScrollArea = NEW Button(font, x, y, w, h - w - w);
    m_ScrollArea->setDepth(depth - 0.002f);
    m_ScrollArea->setPaddingSize(default_padding);
    m_ScrollArea->setColor(Factions::Database[FactionEnum::Federation].GUIColor);

    m_ScrollArea->setTextureCorner(ResourceManifest::GUITextureCornerBoxSmall);
    m_ScrollArea->setTextureEdge(ResourceManifest::GUITextureSideSmall);
    m_ScrollArea->setTextureCornerHighlight(ResourceManifest::GUITextureCornerBoxSmall);
    m_ScrollArea->setTextureEdgeHighlight(ResourceManifest::GUITextureSideSmall);

    m_ScrollArea->setAlignment(Alignment::TopLeft);
    m_ScrollArea->setUserPointer(this);
    m_ScrollArea->setOnClickFunctor(ScrollBar_Functor());

    m_ScrollAreaBackground = NEW Button(font, x, y, w, h - w - w);
    m_ScrollAreaBackground->setDepth(depth + 0.001f);
    m_ScrollAreaBackground->setPaddingSize(default_padding);
    m_ScrollAreaBackground->setColor(Factions::Database[FactionEnum::Federation].GUIColorSlightlyDarker);
    m_ScrollAreaBackground->setAlignment(Alignment::TopLeft);
    m_ScrollAreaBackground->disableMouseover();
    m_ScrollAreaBackground->setTextureCorner(ResourceManifest::GUITextureCornerBoxSmall);
    m_ScrollAreaBackground->setTextureEdge(ResourceManifest::GUITextureSideSmall);
    m_ScrollAreaBackground->setTextureCornerHighlight(ResourceManifest::GUITextureCornerBoxSmall);
    m_ScrollAreaBackground->setTextureEdgeHighlight(ResourceManifest::GUITextureSideSmall);

    m_TopOrLeftButton     = NEW Button(font, x,y, w, w);
    m_TopOrLeftButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_TopOrLeftButton->setPaddingSize(default_padding);
    m_TopOrLeftButton->setDepth(depth - 0.001f);
    m_TopOrLeftButton->setText("");
    m_TopOrLeftButton->setAlignment(Alignment::TopLeft);
    m_TopOrLeftButton->setTextColor(0, 0, 0, 1);
    m_TopOrLeftButton->setOnClickToBePulsed(true);
    m_TopOrLeftButton->setUserPointer(this);
    m_TopOrLeftButton->setOnClickFunctor(ScrollBar_TopButtonFunctor());

    m_TopOrLeftButton->setTextureEdge(ResourceManifest::GUITextureSideSmall);
    m_TopOrLeftButton->setTextureCorner(ResourceManifest::GUITextureCornerRoundSmall);
    m_TopOrLeftButton->setTextureEdgeHighlight(ResourceManifest::GUITextureSideSmall);
    m_TopOrLeftButton->setTextureCornerHighlight(ResourceManifest::GUITextureCornerRoundSmall);

    m_TopOrLeftButton->setTextureCorner(ResourceManifest::GUITextureCornerBoxSmall,1);
    m_TopOrLeftButton->setTextureCorner(ResourceManifest::GUITextureCornerBoxSmall,3);
    m_TopOrLeftButton->setTextureCornerHighlight(ResourceManifest::GUITextureCornerBoxSmall, 1);
    m_TopOrLeftButton->setTextureCornerHighlight(ResourceManifest::GUITextureCornerBoxSmall, 3);

    m_BottomOrRightButton = NEW Button(font, x, y - h, w, w);
    m_BottomOrRightButton->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_BottomOrRightButton->setPaddingSize(default_padding);
    m_BottomOrRightButton->setDepth(depth - 0.001f);
    m_BottomOrRightButton->setText("");
    m_BottomOrRightButton->setAlignment(Alignment::BottomLeft);
    m_BottomOrRightButton->setTextColor(0, 0, 0, 1);
    m_BottomOrRightButton->setOnClickToBePulsed(true);
    m_BottomOrRightButton->setUserPointer(this);
    m_BottomOrRightButton->setOnClickFunctor(ScrollBar_BottomButtonFunctor());

    m_BottomOrRightButton->setTextureEdge(ResourceManifest::GUITextureSideSmall);
    m_BottomOrRightButton->setTextureCorner(ResourceManifest::GUITextureCornerRoundSmall);
    m_BottomOrRightButton->setTextureEdgeHighlight(ResourceManifest::GUITextureSideSmall);
    m_BottomOrRightButton->setTextureCornerHighlight(ResourceManifest::GUITextureCornerRoundSmall);

    m_BottomOrRightButton->setTextureCorner(ResourceManifest::GUITextureCornerBoxSmall, 0);
    m_BottomOrRightButton->setTextureCorner(ResourceManifest::GUITextureCornerBoxSmall, 2);
    m_BottomOrRightButton->setTextureCornerHighlight(ResourceManifest::GUITextureCornerBoxSmall, 0);
    m_BottomOrRightButton->setTextureCornerHighlight(ResourceManifest::GUITextureCornerBoxSmall, 2);

    update_scroll_bar_position();
}
ScrollBar::~ScrollBar() {
    SAFE_DELETE(m_ScrollArea);
    SAFE_DELETE(m_ScrollAreaBackground);
    SAFE_DELETE(m_TopOrLeftButton);
    SAFE_DELETE(m_BottomOrRightButton);
}
void ScrollBar::resetScrollOffset() {
    m_ScrollBarCurrentPosition = 0.0f;
    m_DragSnapshot = 0.0f;
    m_ScrollBarStartAnchor = get_scroll_bar_starting_y();
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
    m_ScrollAreaBackground->setSize(m_Width, m_ScrollAreaBackground->height());
    m_TopOrLeftButton->setSize(m_Width, m_Width);
    m_BottomOrRightButton->setSize(m_Width, m_Width);
}
void ScrollBar::setHeight(const float height) {
    Widget::setHeight(height);
    m_ScrollAreaBackground->setSize(m_Width, height - m_Width - m_Width);
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
    m_ScrollAreaBackground->setPosition(x,y - (m_Width));

    update_scroll_bar_position();
}
void ScrollBar::setPosition(const glm::vec2& position) {
    ScrollBar::setPosition(position.x, position.y);
}
void ScrollBar::setSliderSize(const float percent) {
    m_ScrollBarCurrentContentPercent = glm::clamp(percent,0.01f,1.0f);
    update_scroll_bar_position();
}
const bool ScrollBar::isScrollable() const {
    return (m_ScrollBarCurrentContentPercent < 1.0f) ? true : false;
}
const float ScrollBar::get_scroll_area_max_height() const {
    return (m_Height - (m_Width * 2.0f));
}
const float ScrollBar::getSliderPosition() const {
    return m_ScrollBarCurrentPosition;
}
const float ScrollBar::getSliderHeight() const {
    auto res   = (m_ScrollBarCurrentContentPercent) * get_scroll_area_max_height();
    return res;
}
void ScrollBar::setType(const ScrollBarType::Type& type) {
    m_Type = type;
}
void ScrollBar::scroll(const float amount) {
    if (m_ScrollBarCurrentContentPercent < 1.0f) {
        m_ScrollBarCurrentPosition += amount;
        
        const float scrollBarHeight    = getSliderHeight();
        const float starting_y         = get_scroll_bar_starting_y();
        const float bottomMark         = get_scroll_area_max_height() - scrollBarHeight;

        update_scroll_bar_position();

        if (m_ScrollBarStartAnchor > starting_y) {
            m_ScrollBarCurrentPosition = 0.0f;
            m_ScrollBarStartAnchor = starting_y;
        }else if (m_ScrollBarStartAnchor < starting_y - bottomMark) {
            m_ScrollBarCurrentPosition = -bottomMark;
            m_ScrollBarStartAnchor = starting_y + -bottomMark;
        }
    }
}
void ScrollBar::setScrollBarColor(const glm::vec4& color) {
    m_ScrollBarColor = color;
}
const float ScrollBar::get_scroll_bar_starting_y() const {
    return m_Position.y - m_Width;
}
void ScrollBar::update_scroll_bar_position() {
    m_ScrollBarStartAnchor = get_scroll_bar_starting_y() + m_ScrollBarCurrentPosition;
}
void ScrollBar::update(const double& dt) {
    Widget::update(dt);
    m_TopOrLeftButton->update(dt);
    m_BottomOrRightButton->update(dt);
    m_ScrollArea->update(dt);
    m_ScrollAreaBackground->update(dt);

    if (m_Hidden)
        return;
    float scrollBarHeight   = getSliderHeight();
    const float scrollBarY  = m_ScrollBarStartAnchor - scrollBarHeight;
    const auto& mouse       = Engine::getMousePosition();

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
    const float& scrollBarHeight = getSliderHeight();
    const auto pos = positionWorld();

    if (m_CurrentlyDragging) {
        m_ScrollArea->setColor(Factions::Database[FactionEnum::Federation].GUIColorHighlight);
        m_ScrollArea->setColorHighlight(Factions::Database[FactionEnum::Federation].GUIColorHighlight);
    }else{
        m_ScrollArea->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
        m_ScrollArea->setColorHighlight(Factions::Database[FactionEnum::Federation].GUIColorHighlight);
    }
    m_ScrollArea->setSize(m_Width, scrollBarHeight);
    m_ScrollArea->setPosition(pos.x, m_ScrollBarStartAnchor);

    m_ScrollAreaBackground->render();
    m_ScrollArea->render();
    m_TopOrLeftButton->render();
    m_BottomOrRightButton->render();

    Widget::render(scissor);
}
void ScrollBar::render() {
    ScrollBar::render(glm::vec4(-1.0f));
}
#include "RoundedWindow.h"
#include <core/engine/renderer/Renderer.h>
#include "../factions/Faction.h"

#include "Text.h"

using namespace std;
using namespace Engine;

struct emptyPosFunctor { glm::vec2 operator()(RoundedWindow* t) const {
    return t->positionLocal();
} };
struct emptySizeFunctor { glm::vec2 operator()(RoundedWindow* t) const {
    return glm::vec2(t->width(), t->height());
}};

RoundedWindow::RoundedWindow(Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth,const unsigned int& borderSize, const string& labelText) : Button(font, x,y, width, height) {
    m_BorderSize = borderSize;
    setSizeFunctor(emptySizeFunctor());
    setPositionFunctor(emptyPosFunctor());
    
    m_Label = NEW Text(0, (height / 2.0f) - 10.0f, font, labelText);
    m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_Label->setAlignment(Alignment::Center);
    m_Label->setTextAlignment(TextAlignment::Center);

    m_Background = NEW Button(font, 0, 0, (width)+(borderSize*2.0f), (height)+(borderSize * 2.0f));
    m_Background->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_Background->setDepth(depth + 0.01f);
    m_Background->disable();

    setColor(Factions::Database[FactionEnum::Federation].GUIColorDark);
    setDepth(depth);
    disable();
    
    addChild(m_Background);
    addChild(m_Label);
}
RoundedWindow::~RoundedWindow() {

}
void RoundedWindow::setAlignment(const Alignment::Type& alignment) {
    Widget::setAlignment(alignment);
    m_Background->setAlignment(alignment);
}
void RoundedWindow::setLabelText(const string& text) {
    m_Label->setText(text);
}
void RoundedWindow::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    const auto element_size = m_SizeFunctor();

    setSize(element_size.x, element_size.y);
    const auto element_pos = m_PositionFunctor();
    setPosition(element_pos.x, element_pos.y);

    m_Label->setPosition(0, (element_size.y / 2.0f) - 10.0f);
    m_Background->setSize(element_size.x + (static_cast<float>(m_BorderSize) * 2.0f), element_size.y + (static_cast<float>(m_BorderSize) * 2.0f));
}
void RoundedWindow::update(const double& dt) {
    Button::update(dt);
}
void RoundedWindow::render() {
    Button::render();
    const auto pos = positionWorld();
    const float width_ = width();
    const float height_ = height();
    Renderer::renderRectangle(
        pos + glm::vec2(0, (height_ / 2.0f) - 50.0f),
        Factions::Database[FactionEnum::Federation].GUIColor,
        width_ - 60.0f,
        1,
        0.0f,
        getDepth() - 0.01f
    );
}
#include "ScrollWindow.h"
#include "ScrollBar.h"
#include "Text.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/fonts/Font.h>

using namespace Engine;
using namespace std;

ScrollFrame::ScrollFrame(const float x, const float y, const float w, const float h) : Widget(x, y, w, h) {
    m_BorderSize = 1.0f;

    m_ScrollBar = new ScrollBar(x + w, y, 20.0f, h);

    setContentPadding(10.0f);
    m_ContentHeight = 0.0f;
    setPosition(m_Position.x, m_Position.y);
}
ScrollFrame::~ScrollFrame() {
    SAFE_DELETE(m_ScrollBar);
    SAFE_DELETE_VECTOR(m_Content);
}
void ScrollFrame::internalAddContent() {
    float height = 0;
    for (auto& widget : m_Content) {
        height += widget->height();
        height += m_ContentPadding;
    }
    m_ContentHeight = height;
    const float& percent = m_Height / m_ContentHeight;
    m_ScrollBar->setSliderSize(percent);
}
void ScrollFrame::setAlignment(const Alignment::Type& alignment) {
    m_Alignment = alignment;
    setPosition(m_Position.x, m_Position.y);
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
        widget->setPosition(m_Position.x + 1.0f, m_Position.y - (m_ContentPadding / 2.0f));
    }else{
        widget->setPosition(m_Position.x + 1.0f, (m_Position.y - (m_ContentPadding / 2.0f)) + (m_ContentHeight + m_ContentPadding));
    }

    //modify text if needed
    Text* textWidget = dynamic_cast<Text*>(widget);
    if (textWidget) {
        float textwidth = 0;
        unsigned int count = 0;
        auto text = textWidget->text();
        for (auto itr = text.begin(); itr != text.end(); ++itr) {
            auto character = (*itr);
            if (character != '\0' && character != '\n') {
                textwidth += (textWidget->font().getGlyphData(character).width) * textWidget->textScale().x;
                if (textwidth > m_Width - 110.0f) {
                    itr = text.insert(itr, '\n');
                    textwidth = 0.0f;
                }
            }
            ++count;
        }
        textWidget->setText(text);
    }
    m_Content.push_back(widget);
    internalAddContent();
}

void ScrollFrame::setPosition(const float x, const float y) {
    m_ScrollBar->setPosition(x + m_Width, y);
    Widget::setPosition(x, y);
}
void ScrollFrame::setPosition(const glm::vec2& position) {
    m_ScrollBar->setPosition(position.x, position.y);
    Widget::setPosition(position.x, position.y);
}

void ScrollFrame::setColor(const glm::vec4& color) {
    m_ScrollBar->setColor(color);
    Widget::setColor(color);
}
void ScrollFrame::setContentPadding(const float padding) {
    m_ContentPadding = padding;
}
void ScrollFrame::update(const double& dt) {
    Widget::update(dt);
    m_ScrollBar->update(dt);

    auto height = 0.0f;
    auto scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2.0f)));
    for (auto& widget : m_Content) {
        widget->setPosition(m_Position.x + 1.0f, ((m_Position.y - (m_ContentPadding / 2.0f)) - height) - scrollOffset);
        height += widget->height();
        height += m_ContentPadding;
        widget->update(dt);
    }

    if (m_MouseIsOver || m_ScrollBar->isMouseOver()) {
        const auto delta = Engine::getMouseWheelDelta();
        if (delta != 0.0) {
            m_ScrollBar->scroll(static_cast<float>(delta * 0.2));
        }
    }

}
void ScrollFrame::render(const glm::vec4& scissor) {
    m_ScrollBar->render(scissor);

    const auto pos = positionWorld();

    Renderer::renderBorder(m_BorderSize, glm::vec2(pos.x + 1.0f, pos.y), m_Color, m_Width, m_Height, 0, 0.02f, m_Alignment, scissor);

    //content background
    auto scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2.0f)));
    Renderer::renderRectangle(glm::vec2(pos.x, pos.y - scrollOffset), glm::vec4(0.3f), m_Width, m_ContentHeight, 0, 0.021f, m_Alignment, scissor);

    for (auto& widget : m_Content) {
        widget->render(scissor);
    }

    Widget::render(scissor);
}
void ScrollFrame::render() {
    m_ScrollBar->render();

    const auto pos = positionWorld();

    Renderer::renderBorder(m_BorderSize, glm::vec2(pos.x + 1.0f, pos.y), m_Color, m_Width, m_Height, 0, 0.02f, m_Alignment);

    const auto scissor = glm::vec4(pos.x, pos.y - m_Height, m_Width, m_Height);

    //content background
    auto scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2.0f)));
    Renderer::renderRectangle(glm::vec2(pos.x, pos.y - scrollOffset), glm::vec4(0.3f), m_Width, m_ContentHeight, 0, 0.021f, m_Alignment, scissor);
   
    for (auto& widget : m_Content) {
        widget->render(scissor);
    }
    Widget::render(scissor);
}
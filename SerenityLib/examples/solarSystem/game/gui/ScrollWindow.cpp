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

    const auto scrollbar_width = 20.0f;

    m_ScrollBar = NEW ScrollBar(x + w - scrollbar_width, y, scrollbar_width, h);

    setContentPadding(10.0f);
    m_ContentHeight = 0.0f;
    setPosition(m_Position.x, m_Position.y);
}
ScrollFrame::~ScrollFrame() {
    SAFE_DELETE(m_ScrollBar);
    SAFE_DELETE_VECTOR(m_Content);
}
void ScrollFrame::addContent(Widget* widget) {
    if (m_Content.size() == 0) {
        widget->setPosition(m_Position.x + 1.0f,  m_Position.y - (m_ContentPadding / 2.0f));
    }else{
        widget->setPosition(m_Position.x + 1.0f, (m_Position.y - (m_ContentPadding / 2.0f)) + (m_ContentHeight + m_ContentPadding));
    }
    m_Content.push_back(widget);
    internal_recalculate_content_sizes();
}
void ScrollFrame::fit_widget_to_window(Widget* widget) {
    if (!widget)
        return;
    //goal: shrink the width of the widget to fit the window if it overflows. possibly increase the height of the widget if it is a text widget, etc
    Text* textWidget = dynamic_cast<Text*>(widget);

    float actual_content_width = 0.0f;
    if (m_ScrollBar->isScrollable()) {
        actual_content_width = m_Width - m_ScrollBar->width();
    }else{
        actual_content_width = m_Width;
    }


    if (textWidget) {
        const auto threshold = actual_content_width - 110.0f; //TODO: re-eval this hardcoded number
        float text_width = 0.0f;
        bool changed = false;
        string text = textWidget->text();
        for (auto itr = text.begin(); itr != text.end(); ++itr) {
            auto character = (*itr);
            if (character != '\0' && character != '\n') {
                text_width += (textWidget->font().getGlyphData(character).width) * textWidget->textScale().x;
                if (text_width > threshold) {
                    itr = text.insert(itr, '\n');
                    text_width = 0.0f;
                    changed = true;
                }
            }
            if (character == '\n')
                text_width = 0.0f;
        }
        if (changed)
            textWidget->setText(text);

        if (m_ScrollBar->isScrollable()) {

        }else {

        }
    }else{
        //just shrink width if needed
        widget->setWidth((widget->width() > actual_content_width - 1) ? actual_content_width : widget->width());
    }
}
void ScrollFrame::internal_recalculate_content_sizes() {
    float height = 0;
    float width_accumulator = 0;

    float actual_content_width = 0;
    if (m_ScrollBar->isScrollable()) {
        actual_content_width = m_Width - m_ScrollBar->width();
    }else {
        actual_content_width = m_Width;
    }

    for (size_t i = 0; i < m_Content.size(); ++i) {
        auto& widget            = *m_Content[i];
        fit_widget_to_window(&widget);
        width_accumulator += widget.width();
        if (width_accumulator >= actual_content_width - 1) {
            height += (widget.height() + m_ContentPadding);
            width_accumulator = 0;
        }
    }
    m_ContentHeight = height;
    const float percent = m_Height / m_ContentHeight;
    m_ScrollBar->setSliderSize(percent);
}
void ScrollFrame::setAlignment(const Alignment::Type& alignment) {
    m_Alignment = alignment;
    setPosition(m_Position.x, m_Position.y);
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
    internal_recalculate_content_sizes();
}

void ScrollFrame::setWidth(const float w) {
    Widget::setWidth(w);
    internal_recalculate_content_sizes();
}
void ScrollFrame::setHeight(const float h) {
    Widget::setHeight(h);
    internal_recalculate_content_sizes();
}
void ScrollFrame::setSize(const float width, const float height) {
    Widget::setSize(width, height);
    internal_recalculate_content_sizes();
}

void ScrollFrame::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ScrollBar->setColor(r, g, b, a);
    Widget::setColor(r, g, b, a);

}
void ScrollFrame::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    internal_recalculate_content_sizes();
}
void ScrollFrame::removeContent(Widget* widget) {
    SAFE_DELETE(widget);
    removeFromVector(m_Content, widget);
    internal_recalculate_content_sizes();
}
void ScrollFrame::setPosition(const float x, const float y) {
    m_ScrollBar->setPosition(x + m_Width - m_ScrollBar->width(), y);
    Widget::setPosition(x, y);
}
void ScrollFrame::setPosition(const glm::vec2& position) {
    ScrollFrame::setPosition(position.x, position.y);
}

void ScrollFrame::setColor(const glm::vec4& color) {
    m_ScrollBar->setColor(color);
    Widget::setColor(color);
}
void ScrollFrame::setContentPadding(const float padding) {
    m_ContentPadding = padding;
    internal_recalculate_content_sizes();
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
    if (m_ScrollBar->isScrollable())
        m_ScrollBar->render(scissor);

    const auto pos = positionWorld();

    Renderer::renderBorder(m_BorderSize, glm::vec2(pos.x + 1.0f, pos.y), m_Color, m_Width + 1.0f, m_Height, 0, 0.02f, m_Alignment, scissor);

    //content background
    auto scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2.0f)));
    Renderer::renderRectangle(glm::vec2(pos.x, pos.y - scrollOffset), glm::vec4(0.3f), m_Width + 1.0f, m_ContentHeight, 0, 0.021f, m_Alignment, scissor);

    for (auto& widget : m_Content) {
        widget->render(scissor);
    }

    Widget::render(scissor);
}
void ScrollFrame::render() {
    if(m_ScrollBar->isScrollable())
        m_ScrollBar->render();

    const auto pos = positionWorld();

    Renderer::renderBorder(m_BorderSize, glm::vec2(pos.x + 1.0f, pos.y), m_Color, m_Width + 1.0f, m_Height, 0, 0.02f, m_Alignment);

    const auto scissor = glm::vec4(pos.x, pos.y - m_Height, m_Width + 1, m_Height);

    //content background
    auto scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2.0f)));
    Renderer::renderRectangle(glm::vec2(pos.x, pos.y - scrollOffset), glm::vec4(0.3f), m_Width + 1.0f, m_ContentHeight, 0, 0.021f, m_Alignment, scissor);
   
    for (auto& widget : m_Content) {
        widget->render(scissor);
    }
    Widget::render(scissor);
}
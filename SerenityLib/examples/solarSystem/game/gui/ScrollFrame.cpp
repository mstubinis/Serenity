#include "ScrollFrame.h"
#include "ScrollBar.h"
#include "Text.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/fonts/Font.h>

using namespace Engine;
using namespace std;

const auto scrollbar_width = 30.0f;


ScrollFrame::WidgetEntry::WidgetEntry() {
    widget = nullptr;
    original_width = 0.0f;
    original_text = "";
}
ScrollFrame::WidgetEntry::WidgetEntry(Widget* widget_) {
    widget = widget_;
    original_width = widget_->width();
    Text* text = dynamic_cast<Text*>(widget_);
    if (text) {
        original_text = text->text();
    }
}
ScrollFrame::WidgetEntry::~WidgetEntry() {
}


ScrollFrame::WidgetRow::WidgetRow() {
    maxHeight = 0.0f;
}
ScrollFrame::WidgetRow::~WidgetRow() {
}
void ScrollFrame::WidgetRow::clear() {
    for (auto& widgetEntry : widgets) {
        SAFE_DELETE(widgetEntry.widget);
    }
    maxHeight = 0.0f;
}



ScrollFrame::ScrollFrame(const Font& font, const float x, const float y, const float w, const float h, const float depth) : Widget(x, y, w, h) {
    m_ScrollBar = NEW ScrollBar(font, x + w - scrollbar_width, y, scrollbar_width, h, depth - 0.001f);

    m_ContentHeight = 0.0f;
    setPosition(m_Position.x, m_Position.y);
}
ScrollFrame::~ScrollFrame() {
    SAFE_DELETE(m_ScrollBar);
    for (auto& row : m_Content) {
        for (auto& entry : row.widgets) {
            SAFE_DELETE(entry.widget);
        }
    }
}
void ScrollFrame::clear() {
    for (auto& row : m_Content) {
        row.clear();
    }
    m_Content.clear();
    internal_recalculate_content_sizes();
}
void ScrollFrame::addContent(Widget* widget, const unsigned int row) {
    if (m_Content.size() <= row) {
        m_Content.resize(row + 1, ScrollFrame::WidgetRow());
    }
    auto& row_ = m_Content[row];
    WidgetEntry entry(widget);
    row_.widgets.push_back(std::move(entry));
    float max_height = 0.0f;
    for (auto& widgetInRow : row_.widgets) {
        const auto widgetHeight = widgetInRow.widget->height();
        if (widgetHeight > max_height) {
            max_height = widgetHeight;
        }
    }
    row_.maxHeight = max_height;
    internal_recalculate_content_sizes();
}
void ScrollFrame::addContent(Widget* widget) {
    WidgetRow row;
    WidgetEntry entry(widget);
    row.widgets.push_back(std::move(entry));
    row.maxHeight = widget->height();
    m_Content.push_back(std::move(row));
    internal_recalculate_content_sizes();
}
const float ScrollFrame::get_true_content_height(const bool updateToo, const double& dt) {
    float height = 0;
    float actual_content_width = 0;
    const auto scrollbar_width = m_ScrollBar->width();
    if (m_ScrollBar->isScrollable()) {
        actual_content_width = m_Width - scrollbar_width;
    }else{
        actual_content_width = m_Width;
    }
    auto scrollOffset = m_ScrollBar->getSliderPosition() * (m_ContentHeight / (m_Height - (m_ScrollBar->width() * 2.0f)));
    auto pos = positionFromAlignmentWorld();
    for (auto& row : m_Content) {
        float width_accumulator = 0.0f;
        for (auto& widgetEntry : row.widgets) {
            if (updateToo) {
                widgetEntry.widget->setPosition(pos.x + width_accumulator, ((pos.y + m_Height) - height) - scrollOffset);
                widgetEntry.widget->update(dt);
            }
            width_accumulator += widgetEntry.widget->width();
            if (width_accumulator > actual_content_width) {
                width_accumulator = 0.0f;
                height += row.maxHeight;
                if (updateToo) {
                    widgetEntry.widget->setPosition(pos.x + width_accumulator, ((pos.y + m_Height) - height) - scrollOffset);
                    widgetEntry.widget->update(dt);
                }
                width_accumulator += widgetEntry.widget->width();
            }
        }
        height += row.maxHeight;
    }
    return height;
}
void ScrollFrame::fit_widget_to_window(WidgetEntry& widgetEntry) {
    if (!widgetEntry.widget)
        return;
    //goal: shrink the width of the widget to fit the window if it overflows. possibly increase the height of the widget if it is a text widget, etc
    Text* textWidget = dynamic_cast<Text*>(widgetEntry.widget);

    float actual_content_width = 0.0f;
    bool scrollable = m_ScrollBar->isScrollable();
    if (scrollable) {
        actual_content_width = m_Width - m_ScrollBar->width();
    }else{
        actual_content_width = m_Width;
    }
    if (textWidget) {
        const auto threshold = actual_content_width - 240.0f; //TODO: re-eval this hardcoded number
        float text_width = 0.0f;
        bool changed = false;
        string text = widgetEntry.original_text;
        textWidget->setText(text);
        for (auto itr = text.begin(); itr != text.end(); ++itr) {
            auto character = (*itr);
            if (character == '\n')
                text_width = 0.0f;
            else if (character != '\0' && character != '\n') {
                text_width += (textWidget->font().getGlyphData(character).width) * textWidget->textScale().x;
                if (text_width > threshold) {

                    auto itr2 = itr;
                    while (itr2 > text.begin()) {
                        auto character2 = (*itr2);
                        if (character2 == ' ') {
                            text = text.replace(itr2, itr2+1,"\n");
                            text_width = 0.0f;
                            changed = true;
                            break;
                        }
                        --itr2;
                    }
                }
            }
        }
        if (changed) {
            textWidget->setText(text);
            /*
            m_ContentHeight = get_true_content_height();
            const float percent = m_Height / m_ContentHeight;
            m_ScrollBar->setSliderSize(percent);
            m_ScrollBar->resetScrollOffset();
            */
        }
    }else{
        //just shrink width if needed     
        float amount = 0.0f;
        if (scrollable) {
            if ((widgetEntry.widget->width() > actual_content_width)) {
                amount = actual_content_width;
            }else{
                amount = widgetEntry.widget->width();
            }
        }else{
            amount = widgetEntry.original_width;
        }
        widgetEntry.widget->setWidth(amount);
    }
}
void ScrollFrame::internal_recalculate_content_sizes() {
    m_ContentHeight = get_true_content_height();
    float percent = m_Height / m_ContentHeight;
    m_ScrollBar->setSliderSize(percent);

    for (auto& row : m_Content) {
        for (auto& widgetEntry : row.widgets) {
            fit_widget_to_window(widgetEntry);
        }
    }

    m_ScrollBar->resetScrollOffset();
}
void ScrollFrame::setAlignment(const Alignment::Type& alignment) {
    m_Alignment = alignment;
    setPosition(m_Position.x, m_Position.y);
}

vector<ScrollFrame::WidgetRow>& ScrollFrame::content() {
    return m_Content;
}
const float ScrollFrame::contentHeight() const {
    return m_ContentHeight;
}
void ScrollFrame::reposition_scroll_bar() {
    const auto pos = positionFromAlignmentWorld();
    m_ScrollBar->setPosition(pos.x + m_Width - m_ScrollBar->width(), pos.y + m_Height);
    m_ScrollBar->setSize(m_ScrollBar->width(), m_Height);
}
void ScrollFrame::setWidth(const float w) {
    Widget::setWidth(w);
    reposition_scroll_bar();
    internal_recalculate_content_sizes();
}
void ScrollFrame::setHeight(const float h) {
    Widget::setHeight(h);
    reposition_scroll_bar();
    internal_recalculate_content_sizes();
}
void ScrollFrame::setSize(const float width, const float height) {
    Widget::setSize(width, height);
    reposition_scroll_bar();
    internal_recalculate_content_sizes();
}

void ScrollFrame::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_ScrollBar->setColor(r, g, b, a);
    Widget::setColor(r, g, b, a);

}
void ScrollFrame::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    reposition_scroll_bar();
    internal_recalculate_content_sizes();
}
void ScrollFrame::removeContent(Widget* widget) {
    size_t index = 0;
    for (auto& row : m_Content) {
        for (size_t i = 0; i < row.widgets.size(); ++i) {
            if (row.widgets[i].widget == widget) {
                index = i;
                break;
            }
        }
        row.widgets.erase(row.widgets.begin() + index);
    }
    //TODO: erase empty rows?
    SAFE_DELETE(widget);
    internal_recalculate_content_sizes();
}
void ScrollFrame::setPosition(const float x, const float y) {
    Widget::setPosition(x, y);
    reposition_scroll_bar();
}
void ScrollFrame::setPosition(const glm::vec2& position) {
    ScrollFrame::setPosition(position.x, position.y);
}

void ScrollFrame::setColor(const glm::vec4& color) {
    m_ScrollBar->setColor(color);
    Widget::setColor(color);
}
void ScrollFrame::update(const double& dt) {
    Widget::update(dt);
    m_ScrollBar->update(dt);

    get_true_content_height(true, dt);

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

    //Renderer::renderBorder(1, pos, m_Color, m_Width, m_Height, 0, 0.0001f, m_Alignment, scissor);

    for (auto& row : m_Content) {
        for (auto& widgetEntry : row.widgets) {
            widgetEntry.widget->render(scissor);
        }
    }
    Widget::render(scissor);
}
void ScrollFrame::render() {
    if(m_ScrollBar->isScrollable())
        m_ScrollBar->render();

    const auto pos  = positionWorld();
    const auto pos2 = positionFromAlignmentWorld();

    //Renderer::renderBorder(1, pos, m_Color, m_Width, m_Height, 0, 0.0001f, m_Alignment);

    auto scissor = glm::vec4(pos2.x, pos2.y, m_Width, m_Height);
    //auto scissor = glm::vec4(-1.0f);
    for (auto& row : m_Content) {
        for (auto& widgetEntry : row.widgets) {
            widgetEntry.widget->render(scissor);
        }
    }
    Widget::render(scissor);
}
#include "Button.h"


#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/textures/Texture.h>

#include "../ResourceManifest.h"

using namespace Engine;
using namespace std;

namespace Engine {
    namespace epriv {
        struct emptyFunctor { template<class T> void operator()(T* r) const {} };
    };
};
void Button::setTextureHighlight(Texture* texture) {
    m_RenderElement.setTextureHighlight(texture);
}
void Button::setTextureCornerHighlight(Texture* texture) {
    m_RenderElement.setTextureCornerHighlight(texture);
}
void Button::setTextureEdgeHighlight(Texture* texture) {
    m_RenderElement.setTextureEdgeHighlight(texture);
}
void Button::setTextureHighlight(Handle& handle) {
    m_RenderElement.setTextureHighlight(handle);
}
void Button::setTextureCornerHighlight(Handle& handle) {
    m_RenderElement.setTextureCornerHighlight(handle);
}
void Button::setTextureEdgeHighlight(Handle& handle) {
    m_RenderElement.setTextureEdgeHighlight(handle);
}
void Button::setTexture(Texture* texture) {
    m_RenderElement.setTexture(texture);
}
void Button::setTextureCorner(Texture* texture) {
    m_RenderElement.setTextureCorner(texture);
}
void Button::setTextureEdge(Texture* texture) {
    m_RenderElement.setTextureEdge(texture);
}
void Button::setTexture(Handle& handle) {
    m_RenderElement.setTexture(handle);
}
void Button::setTextureCorner(Handle& handle) {
    m_RenderElement.setTextureCorner(handle);
}
void Button::setTextureEdge(Handle& handle) {
    m_RenderElement.setTextureEdge(handle);
}
void Button::setWidth(const float w) {
    Widget::setWidth(w);
}
void Button::setHeight(const float h) {
    Widget::setHeight(h);
}
void Button::setSize(const float width, const float height) {
    Widget::setSize(width, height);
}
Button::Button(const Font& font, const float x, const float y, const float width, const float height) : Widget(x, y, width, height), m_RenderElement(*this){
    setFont(font);
    setText("Button");
    setOnClickFunctor(epriv::emptyFunctor());
    setTextScale(1.0f, 1.0f);
    m_Padding = 20.0f;
    setAlignment(Alignment::Center);
    setTextAlignment(TextAlignment::Center);

    enable();

    setColor(m_Color);

    setDepth(0.008f);
    m_PulseClicked = false;
    m_RenderElement.enableTexture(true);
    m_RenderElement.setTextureCorner(ResourceManifest::GUITextureCorner);
    m_RenderElement.setTextureEdge(ResourceManifest::GUITextureSide);

    m_RenderElement.setTextureCornerHighlight(ResourceManifest::GUITextureCorner);
    m_RenderElement.setTextureEdgeHighlight(ResourceManifest::GUITextureSide);
}
Button::Button(const Font& font, const glm::vec2& position, const float width, const float height) : Button(font, position.x, position.y, width, height) {
}
Button::~Button() {

}
void Button::enableTexture(const bool enabled) {
    m_RenderElement.enableTexture(enabled);
}
void Button::enableTextureCorner(const bool enabled) {
    m_RenderElement.enableTextureCorner(enabled);
}
void Button::enableTextureEdge(const bool enabled) {
    m_RenderElement.enableTextureEdge(enabled);
}
void Button::setDepth(const float& depth) {
    m_RenderElement.setDepth(depth);
}
void Button::enable() {
    m_Enabled = true;
    m_RenderElement.enableMouseover();
}
void Button::disable() {
    m_Enabled = false;
    m_RenderElement.disableMouseover();
}
const bool Button::enabled() {
    return m_Enabled;
}
const bool Button::disabled() {
    return !m_Enabled;
}
void Button::setTextScale(const float scale) {
    Button::setTextScale(scale, scale);
}
void Button::setTextScale(const glm::vec2& scale) {
    Button::setTextScale(scale.x, scale.y);
}
void Button::setTextScale(const float x, const float y) {
    m_TextScale.x = x;
    m_TextScale.y = y;
}
void Button::setTextAlignment(const TextAlignment::Type& textAlignment) {
    m_TextAlignment = textAlignment;
}
const TextAlignment::Type& Button::getTextAlignment() const {
    return m_TextAlignment;
}
const glm::vec2& Button::getTextScale() const {
    return m_TextScale;
}
const string& Button::text() const {
    return m_Text;
}

const float Button::getTextHeight() const {
    return (m_Font->getTextHeight(m_Text) * m_TextScale.y);
}
const float Button::getTextWidth() const {
    return (m_Font->getTextWidth(m_Text) * m_TextScale.x);
}
const float Button::getCornerWidth() {
    return (m_RenderElement.getTextureCorner()) ? m_RenderElement.getTextureCorner()->width() : 0.0f;
}
const float Button::getCornerHeight() {
    return (m_RenderElement.getTextureCorner()) ? m_RenderElement.getTextureCorner()->height() : 0.0f;
}
void Button::setTextColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_TextColor, r, g, b, a);
}
void Button::setTextColor(const glm::vec4& color) {
    Math::setColor(m_TextColor, color.r, color.g, color.b, color.a);
}
void Button::setColor(const float& r, const float& g, const float& b, const float& a) {
    Widget::setColor(r, g, b, a);
    m_RenderElement.setColor(glm::vec4(r, g, b, a));
    m_RenderElement.setColorHighlight(glm::vec4(r, g, b, a) + glm::vec4(0.25f, 0.25f, 0.25f, 0.0f));
}
void Button::setColor(const glm::vec4& color) {
    Widget::setColor(color);
    m_RenderElement.setColor(color);
    m_RenderElement.setColorHighlight(color + glm::vec4(0.25f, 0.25f, 0.25f, 0.0f));
}
void Button::setFont(const Font& font) {
    m_Font = &const_cast<Font&>(font);
}
void Button::setText(const char* text) {
    m_Text = text;
}
void Button::setText(const string& text) {
    m_Text = text;
}
void Button::setOnClickToBePulsed(const bool pulsed) {
    m_PulseClicked = pulsed;
}
void Button::update(const double& dt) {
    Widget::update(dt);
    if (!m_Hidden) {
        if (m_Enabled) {
            if (!m_PulseClicked) {
                if (m_MouseIsOver == true && Engine::isMouseButtonDownOnce(MouseButton::Left)) {
                    m_FunctorOnClick();
                }
            }else{
                if (m_MouseIsOver == true && Engine::isMouseButtonDown(MouseButton::Left)) {
                    m_FunctorOnClick();
                }
            }
        }
    }
}
void Button::render(const glm::vec4& scissor) {
    if (!m_Hidden) {
        const auto pos = positionWorld();

        m_RenderElement.render(scissor);

        auto corner = m_RenderElement.getEdgeWidth(3) + (m_RenderElement.getCornerWidth() / 2.0f);
        auto corner_width_half = corner / 2.0f;

        auto txt_height = getTextHeight();

        glm::vec2 newPosTxt;
        switch (m_TextAlignment) {
            case TextAlignment::Left: {
                newPosTxt = glm::vec2(pos.x - corner_width_half, pos.y + txt_height);
                break;
            }case TextAlignment::Center: {
                newPosTxt = glm::vec2(pos.x, pos.y + txt_height);
                break;
            }case TextAlignment::Right: {
                newPosTxt = glm::vec2(pos.x + corner_width_half, pos.y + txt_height);
                break;
            }default: {
                newPosTxt = glm::vec2(pos.x, pos.y + txt_height);
                break;
            }
        }
        glm::vec2 f(0.0f);
        Renderer::alignmentOffset(m_Alignment, f.x, f.y, m_Width, m_Height);

        m_Font->renderText(m_Text, glm::vec2(newPosTxt.x + f.x, newPosTxt.y + f.y), m_TextColor, 0, m_TextScale, 0.0077f, m_TextAlignment, scissor);
    }
    Widget::render(scissor);
}
void Button::render() {
    Button::render(glm::vec4(-1.0f));
}
#include "GUIRenderElement.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include "Widget.h"
#include "../ResourceManifest.h"
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/Texture.h>

#include <iostream>

using namespace std;
using namespace Engine;

GUIRenderElement::TextureData::TextureData(){
    texture = nullptr;
    textureHighlight = nullptr;

    glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    glm::vec4 colorHighlight = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);

    bool drawSolidColor = false;
}
GUIRenderElement::TextureData::~TextureData() {

}
void GUIRenderElement::TextureData::render(const float& depth, const bool& mouseOver, const glm::vec2& position, const glm::vec2& scale_or_size, const glm::vec4& scissor, const float& angle, const Alignment::Type& align) {
    if (mouseOver) {
        if (textureHighlight) {
            Renderer::renderTexture(*textureHighlight, position, colorHighlight, angle, scale_or_size, depth, align, scissor);
        }else{
            if (drawSolidColor) {
                Renderer::renderRectangle(position, colorHighlight, scale_or_size.x, scale_or_size.y, 0.0f, depth, align, scissor);
            }
        }
    }else{
        if (texture) {
            Renderer::renderTexture(*texture, position, color, angle, scale_or_size, depth, align, scissor);
        }else{
            if (drawSolidColor) {
                Renderer::renderRectangle(position, color, scale_or_size.x, scale_or_size.y, 0.0f, depth, align, scissor);
            }
        }
    }
}


GUIRenderElement::GUIRenderElement(Widget& owner) : m_Owner(owner) {
    m_Textures.resize(GUIRenderElement::TextureIndex::_TOTAL);
    m_BorderColors.resize(GUIRenderElement::BorderIndex::_TOTAL, glm::vec4(1.0f));

    m_PaddingSize = m_BorderSize = glm::ivec4(0);

    m_Depth          = 0.05f;

    enableMouseover();
}
GUIRenderElement::~GUIRenderElement() {

}
unsigned int GUIRenderElement::get_corner_size(const glm::vec2& total_size) {
    unsigned int ret = 0;
    if (m_Textures[TextureIndex::TopLeft].texture) {
        ret = m_Textures[TextureIndex::TopLeft].texture->width();
    }
    return ret;
}
unsigned int GUIRenderElement::get_left_edge_size(const glm::vec2& total_size) {
    unsigned int ret = 0;
    if (m_Textures[TextureIndex::Left].texture) {
        ret = static_cast<unsigned int>(total_size.y) - (get_corner_size(total_size) * 2);
    }
    return ret;
}
unsigned int GUIRenderElement::get_top_edge_size(const glm::vec2& total_size) {
    unsigned int ret = 0;
    if (m_Textures[TextureIndex::Top].texture) {
        ret = static_cast<unsigned int>(total_size.x) - (get_corner_size(total_size) * 2);
    }
    return ret;
}
void GUIRenderElement::setBorderColor(const float& r, const float& g, const float& b, const float& a) {
    for (size_t i = 0; i < m_BorderColors.size(); ++i) {
        m_BorderColors[i] = glm::vec4(r, g, b, a);
    }
}
void GUIRenderElement::setBorderColor(const float& r, const float& g, const float& b, const float& a, unsigned int index) {
    m_BorderColors[index] = glm::vec4(r, g, b, a);
}
void GUIRenderElement::setBorderColor(const glm::vec4& color) {
    GUIRenderElement::setBorderColor(color.r, color.g, color.b, color.a);
}
void GUIRenderElement::setBorderColor(const glm::vec4& color, unsigned int index) {
    GUIRenderElement::setBorderColor(color, index);
}

void GUIRenderElement::enableMouseover(const bool enabled) {
    m_EnableMouseover = enabled;
}
void GUIRenderElement::disableMouseover() {
    m_EnableMouseover = false;
}

void GUIRenderElement::update(const double& dt) {

}
void GUIRenderElement::render() {
    GUIRenderElement::render(glm::vec4(-1.0f));
}
void GUIRenderElement::render(const glm::vec4& scissor) {
    bool mouseOver = false;
    if (m_Owner.isMouseOver() && m_EnableMouseover) {
        mouseOver = true;
    }

    const glm::vec2 total_size = glm::vec2(m_Owner.width(), m_Owner.height());


    const auto pos = m_Owner.positionFromAlignmentWorld();

    const auto corner_size = get_corner_size(total_size);
    const auto left_edge_size = get_left_edge_size(total_size);
    const auto top_edge_size = get_top_edge_size(total_size);

    m_Textures[TextureIndex::TopLeft].    render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + (corner_size / 2),
        (m_BorderSize.y + m_PaddingSize.y + (corner_size / 2))
    ), glm::vec2(1.0f), scissor, 90.0f, Alignment::Center);
    m_Textures[TextureIndex::BottomLeft]. render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + (corner_size / 2),
        (m_BorderSize.y + m_PaddingSize.y + corner_size + left_edge_size + (corner_size / 2))
    ), glm::vec2(1.0f), scissor, 0.0f, Alignment::Center);
    m_Textures[TextureIndex::TopRight].   render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + corner_size + top_edge_size + (corner_size / 2),
        m_BorderSize.y + m_PaddingSize.y + (corner_size / 2)
    ), glm::vec2(1.0f), scissor, 180.0f, Alignment::Center);
    m_Textures[TextureIndex::BottomRight].render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + corner_size + top_edge_size + (corner_size / 2),
        m_BorderSize.y + m_PaddingSize.y + corner_size + left_edge_size + (corner_size / 2)
    ), glm::vec2(1.0f), scissor, 270.0f, Alignment::Center);



    m_Textures[TextureIndex::Left].render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x,
        (m_BorderSize.y + m_PaddingSize.y + corner_size)
    ), glm::vec2(1.0f, left_edge_size), scissor, 0.0f, Alignment::BottomLeft);
    
    m_Textures[TextureIndex::Right].render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + top_edge_size + corner_size,
        (m_BorderSize.y + m_PaddingSize.y + corner_size)
    ), glm::vec2(1.0f, left_edge_size), scissor, 180.0f, Alignment::BottomLeft);

    m_Textures[TextureIndex::Top].render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + (top_edge_size / 2) + corner_size,
        (m_BorderSize.y + m_PaddingSize.y + left_edge_size + corner_size + (corner_size / 2))
    ), glm::vec2(1.0f, top_edge_size), scissor, 270.0f, Alignment::Center);
    
    m_Textures[TextureIndex::Bottom].render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + (top_edge_size / 2) + corner_size,
        (m_BorderSize.y + m_PaddingSize.y + (corner_size / 2))
    ), glm::vec2(1.0f, top_edge_size), scissor, 90.0f, Alignment::Center);
    
    m_Textures[TextureIndex::Center].render(m_Depth, mouseOver, pos + glm::vec2(
        m_BorderSize.x + m_PaddingSize.x + corner_size,
        m_BorderSize.y + m_PaddingSize.y + corner_size
    ), glm::vec2(total_size.x - (corner_size * 2), total_size.y - (corner_size * 2)), scissor, 0.0f, Alignment::BottomLeft);

    //TODO: left, right, top, bottom borders
    if (m_BorderSize.x > 0) {

    }
    if (m_BorderSize.y > 0) {

    }
    if (m_BorderSize.z > 0) {

    }
    if (m_BorderSize.w > 0) {

    }
}
const float GUIRenderElement::getEdgeWidth(unsigned int index) const {
    index = glm::min(index, 4U);
    if (index == 0 || index == 1) {
        return getCornerWidth();
    }
    const glm::vec2 total_size = glm::vec2(m_Owner.width(), m_Owner.height());
    return total_size.x - (getCornerWidth() * 2);
}
const float GUIRenderElement::getEdgeHeight(unsigned int index) const {
    index = glm::min(index, 4U);
    if (index == 0 || index == 1) {
        const glm::vec2 total_size = glm::vec2(m_Owner.width(), m_Owner.height());
        return total_size.y - (getCornerHeight() * 2);
    }
    return getCornerHeight();
}
const float GUIRenderElement::getCornerWidth(unsigned int index) const {
    index = glm::min(index, 4U);
    return (m_Textures[TextureIndex::TopLeft + index].texture) ? m_Textures[TextureIndex::TopLeft + index].texture->width() : 0.0f;
}
const float GUIRenderElement::getCornerHeight(unsigned int index) const {
    index = glm::min(index, 4U);
    return (m_Textures[TextureIndex::TopLeft + index].texture) ? m_Textures[TextureIndex::TopLeft + index].texture->height() : 0.0f;
}
Texture* GUIRenderElement::getTextureCenter() {
    return m_Textures[TextureIndex::Center].texture;
}
Texture* GUIRenderElement::getTextureCorner(unsigned int index) {
    index = glm::min(index, 4U);
    return m_Textures[TextureIndex::TopLeft + index].texture;
}
Texture* GUIRenderElement::getTextureEdge(unsigned int index) {
    index = glm::min(index, 4U);
    return m_Textures[TextureIndex::Left + index].texture;
}
const float& GUIRenderElement::getDepth() const {
    return m_Depth;
}
void GUIRenderElement::setTextureHighlight(Texture* texture) {
    m_Textures[TextureIndex::Center].textureHighlight = texture;
}
void GUIRenderElement::setTextureCornerHighlight(Texture* texture) {
    for (size_t i = TextureIndex::TopLeft; i <= TextureIndex::BottomRight; ++i) {
        m_Textures[i].textureHighlight = texture;
    }
}
void GUIRenderElement::setTextureEdgeHighlight(Texture* texture) {
    for (size_t i = TextureIndex::Left; i <= TextureIndex::Bottom; ++i) {
        m_Textures[i].textureHighlight = texture;
    }
}
void GUIRenderElement::setTextureHighlight(Handle& handle) {
    GUIRenderElement::setTextureHighlight((Texture*)handle.get());
}
void GUIRenderElement::setTextureCornerHighlight(Handle& handle) {
    GUIRenderElement::setTextureCornerHighlight((Texture*)handle.get());
}
void GUIRenderElement::setTextureEdgeHighlight(Handle& handle) {
    GUIRenderElement::setTextureEdgeHighlight((Texture*)handle.get());
}
void GUIRenderElement::enableTexture(const bool enabled) {
    m_Textures[TextureIndex::Center].drawSolidColor = enabled;
}
void GUIRenderElement::enableTextureCorner(const bool enabled) {
    for (size_t i = TextureIndex::TopLeft; i <= TextureIndex::BottomRight; ++i) {
        m_Textures[i].drawSolidColor = enabled;
    }
}
void GUIRenderElement::enableTextureEdge(const bool enabled) {
    for (size_t i = TextureIndex::Left; i <= TextureIndex::Bottom; ++i) {
        m_Textures[i].drawSolidColor = enabled;
    }
}
void GUIRenderElement::setTexture(Texture* texture) {
    m_Textures[TextureIndex::Center].texture = texture;
}
void GUIRenderElement::setTextureCorner(Texture* texture) {
    for (size_t i = TextureIndex::TopLeft; i <= TextureIndex::BottomRight; ++i) {
        m_Textures[i].texture = texture;
    }
}
void GUIRenderElement::setTextureEdge(Texture* texture) {
    for (size_t i = TextureIndex::Left; i <= TextureIndex::Bottom; ++i) {
        m_Textures[i].texture = texture;
    }
}
void GUIRenderElement::setTexture(Handle& handle) {
    GUIRenderElement::setTexture((Texture*)handle.get());
}
void GUIRenderElement::setTextureCorner(Handle& handle) {
    GUIRenderElement::setTextureCorner((Texture*)handle.get());
}
void GUIRenderElement::setTextureEdge(Handle& handle) {
    GUIRenderElement::setTextureEdge((Texture*)handle.get());
}
void GUIRenderElement::setColor(const glm::vec4& color) {
    for (size_t i = 0; i < m_Textures.size(); ++i) {
        m_Textures[i].color = color;
    }
}
void GUIRenderElement::setColorHighlight(const glm::vec4& color) {
    for (size_t i = 0; i < m_Textures.size(); ++i) {
        m_Textures[i].colorHighlight = color;
    }
}
void GUIRenderElement::setDepth(const float& depth) {
    m_Depth = depth;
}
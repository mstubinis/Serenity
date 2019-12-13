#include "GUIRenderElement.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include "Widget.h"
#include "../ResourceManifest.h"
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/Texture.h>

#include <iostream>

using namespace std;
using namespace Engine;

GUIRenderElement::GUIRenderElement(Widget& owner) : m_Owner(owner) {
    m_TextureCenter          = nullptr;
    m_TextureCorner          = nullptr;
    m_TextureEdge            = nullptr;

    m_TextureCenterHighlight = nullptr;
    m_TextureCornerHighlight = nullptr;
    m_TextureEdgeHighlight   = nullptr;

    m_Color          = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    m_ColorHighlight = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);
    m_ColorBorder    = glm::vec4(1.0f);
    m_Depth          = 0.05f;
    m_BorderSize     = 0;

    m_EdgeHeight     = m_Owner.height();
    m_EdgeWidth      = m_Owner.width();
}
GUIRenderElement::~GUIRenderElement() {

}
void GUIRenderElement::internal_calculate_sizes() {
    const auto total_width   = m_Owner.width();
    const auto total_height  = m_Owner.height();

    const auto corner_width  = (m_TextureCorner) ? m_TextureCorner->width() : 0;
    const auto corner_height = (m_TextureCorner) ? m_TextureCorner->height() : 0;

    m_EdgeWidth = total_width - (static_cast<float>(corner_width) * 2.0f);
    m_EdgeHeight = total_height - (static_cast<float>(corner_height) * 2.0f);
}
void GUIRenderElement::update(const double& dt) {

}
void GUIRenderElement::render() {
    GUIRenderElement::render(glm::vec4(-1.0f));
}
void GUIRenderElement::render(const glm::vec4& scissor) {
    const auto pos    = m_Owner.positionFromAlignmentWorld();
    const float width  = m_Owner.width();
    const float height = m_Owner.height();
    const int corner_width = (m_TextureCorner) ? m_TextureCorner->width() : 0;
    const int corner_height = (m_TextureCorner) ? m_TextureCorner->height() : 0;

    const float half_corner_width = static_cast<float>(corner_width) / 2.0f;
    const float half_corner_height = static_cast<float>(corner_height) / 2.0f;
    glm::vec4 color;

    Texture* center = nullptr;
    Texture* edge   = nullptr;
    Texture* corner = nullptr;
    if (m_Owner.isMouseOver()) {
        color  = m_ColorHighlight;
        center = (m_TextureCenterHighlight) ? m_TextureCenterHighlight : m_TextureCenter;
        edge   = (m_TextureEdgeHighlight)   ? m_TextureEdgeHighlight   : m_TextureEdge;
        corner = (m_TextureCornerHighlight) ? m_TextureCornerHighlight : m_TextureCorner;
    }else{
        color  = m_Color;
        center = m_TextureCenter;
        edge   = m_TextureEdge;
        corner = m_TextureCorner;
    }

    auto render_edges = [&]() {
        //left edge
        Renderer::renderTexture(*edge, pos + glm::vec2(0.0f, corner_height), color, 0.0f, glm::vec2(1.0f, m_EdgeHeight), m_Depth, Alignment::BottomLeft, scissor);
        //right edge
        Renderer::renderTexture(*edge, pos + glm::vec2(m_EdgeWidth + corner_width, corner_height), color, 180.0f, glm::vec2(1.0f, m_EdgeHeight), m_Depth, Alignment::BottomLeft, scissor);
        //top edge
        Renderer::renderTexture(*edge, pos + glm::vec2(width / 2.0f, (corner_height) + m_EdgeHeight + half_corner_height), color, 270.0f, glm::vec2(1.0f, m_EdgeWidth), m_Depth, Alignment::Center, scissor);
        //bottom edge
        Renderer::renderTexture(*edge, pos + glm::vec2(width / 2.0f, half_corner_height), color, 90.0f, glm::vec2(1.0f, m_EdgeWidth), m_Depth, Alignment::Center, scissor);
    };
    auto render_corners = [&]() {
        //top-left corner
        Renderer::renderTexture(*corner, pos + glm::vec2(half_corner_width, (corner_height) + m_EdgeHeight + half_corner_height), color, 270.0f, glm::vec2(1.0f), m_Depth, Alignment::Center, scissor);
        //bottom-left corner
        Renderer::renderTexture(*corner, pos + glm::vec2(half_corner_width, half_corner_height), color, 0.0f, glm::vec2(1.0f), m_Depth, Alignment::Center, scissor);
        //top right corner
        Renderer::renderTexture(*corner, pos + glm::vec2(half_corner_width + m_EdgeWidth + corner_width, half_corner_height), color, 90.0f, glm::vec2(1.0f), m_Depth, Alignment::Center, scissor);
        //bottom right corner
        Renderer::renderTexture(*corner, pos + glm::vec2(half_corner_width + m_EdgeWidth + corner_width, (corner_height) + m_EdgeHeight + half_corner_height), color, 180.0f, glm::vec2(1.0f), m_Depth, Alignment::Center, scissor);
    };
    if (center && corner && edge) {
        Renderer::renderTexture(*center, pos + glm::vec2(corner_width, corner_height), color, 0.0f, glm::vec2(1.0f), m_Depth, Alignment::BottomLeft, scissor);
        render_edges();
        render_corners();
    }else if (center && (!corner || !edge)) {
        Renderer::renderTexture(*center, pos + glm::vec2(corner_width, corner_height), color, 0.0f, glm::vec2(1.0f), m_Depth, Alignment::BottomLeft, scissor);
    }else if(corner && edge){
        Renderer::renderRectangle(pos + glm::vec2(corner_width, corner_height), color, m_EdgeWidth, m_EdgeHeight, 0.0f, m_Depth, Alignment::BottomLeft, scissor);
        render_edges();
        render_corners();
    }else {
        Renderer::renderRectangle(pos + glm::vec2(corner_width, corner_height), color, m_EdgeWidth, m_EdgeHeight, 0.0f, m_Depth, Alignment::BottomLeft, scissor);
    }

    if (m_ColorBorder.a > 0.0f && m_BorderSize > 0) {
        Renderer::renderBorder(static_cast<float>(m_BorderSize), pos + glm::vec2(corner_width, corner_height), m_ColorBorder, m_EdgeWidth, m_EdgeHeight, 0.0f, m_Depth - 0.001f, Alignment::BottomLeft, scissor);
    }
}
const float& GUIRenderElement::getEdgeWidth() const {
    return m_EdgeWidth;
}
const float& GUIRenderElement::getEdgeHeight() const {
    return m_EdgeHeight;
}
const float GUIRenderElement::getCornerWidth() const {
    return (m_TextureCorner) ? m_TextureCorner->width() : 0.0f;
}
const float GUIRenderElement::getCornerHeight() const {
    return (m_TextureCorner) ? m_TextureCorner->height() : 0.0f;
}
Texture* GUIRenderElement::getTextureCenter() {
    return m_TextureCenter;
}
Texture* GUIRenderElement::getTextureCorner() {
    return m_TextureCorner;
}
Texture* GUIRenderElement::getTextureEdge() {
    return m_TextureEdge;
}
const float& GUIRenderElement::getDepth() const {
    return m_Depth;
}
void GUIRenderElement::setTextureHighlight(Texture* texture) {
    m_TextureCenterHighlight = texture;
    internal_calculate_sizes();
}
void GUIRenderElement::setTextureCornerHighlight(Texture* texture) {
    m_TextureCornerHighlight = texture;
    internal_calculate_sizes();
}
void GUIRenderElement::setTextureEdgeHighlight(Texture* texture) {
    m_TextureEdgeHighlight = texture;
    internal_calculate_sizes();
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
void GUIRenderElement::setTexture(Texture* texture) {
    m_TextureCenter = texture;
    internal_calculate_sizes();
}
void GUIRenderElement::setTextureCorner(Texture* texture) {
    m_TextureCorner = texture;
    internal_calculate_sizes();
}
void GUIRenderElement::setTextureEdge(Texture* texture) {
    m_TextureEdge = texture;
    internal_calculate_sizes();
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
    m_Color = color;
}
void GUIRenderElement::setColorHighlight(const glm::vec4& color) {
    m_ColorHighlight = color;
}
void GUIRenderElement::setDepth(const float& depth) {
    m_Depth = depth;
}
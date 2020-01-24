#pragma once
#ifndef GAME_GUI_RENDER_ELEMENT_H
#define GAME_GUI_RENDER_ELEMENT_H

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <core/engine/renderer/RendererIncludes.h>

class  Texture;
class  Widget;
struct Handle;
class GUIRenderElement {
    friend class Widget;

    public: class BorderIndex final { public: enum Index {
        Left,
        Right,
        Top,
        Bottom,
    _TOTAL,};};

    public: class TextureIndex final{ public: enum Index {
        TopLeft,
        BottomLeft,
        TopRight,
        BottomRight,
        Left,
        Right,
        Top,
        Bottom,
        Center,
     _TOTAL,};};

    public: struct TextureData final {

        Texture* texture;
        Texture* textureHighlight;

        glm::vec4 color;
        glm::vec4 colorHighlight;

        bool drawSolidColor;


        TextureData();
        ~TextureData();
        void render(const float& depth, const bool& mouseOver, const glm::vec2& position, const glm::vec2& scale_or_size, const glm::vec4& scissor, const float& angle, const Alignment::Type& align);
    };

    private:
        Widget*                  m_Owner;

        float                    m_Depth;
        bool                     m_EnableMouseover;

        std::vector<TextureData> m_Textures;
        glm::ivec4 m_PaddingSize;
        glm::ivec4 m_BorderSize;
        std::vector<glm::vec4> m_BorderColors;

        int get_corner_size(const glm::vec2& total_size);
        int get_left_edge_size(const glm::vec2& total_size);
        int get_top_edge_size(const glm::vec2& total_size);

        GUIRenderElement();
    public:
        GUIRenderElement(Widget* owner);

        GUIRenderElement(const GUIRenderElement&)                      = delete;
        GUIRenderElement& operator=(const GUIRenderElement&)           = delete;
        GUIRenderElement(GUIRenderElement&& other) noexcept            = delete;
        GUIRenderElement& operator=(GUIRenderElement&& other) noexcept = delete;

        virtual ~GUIRenderElement();

        void setBorderSize(const unsigned int& size);
        void setBorderSize(const unsigned int& size, const unsigned int& index);

        void setPaddingSize(const unsigned int& size);
        void setPaddingSize(const unsigned int& size, const unsigned int& index);

        void setBorderColor(const glm::vec4& color);
        void setBorderColor(const glm::vec4& color, unsigned int index);
        void setBorderColor(const float& r, const float& g, const float& b, const float& a);
        void setBorderColor(const float& r, const float& g, const float& b, const float& a, unsigned int index);

        void enableMouseover(const bool = true);
        void disableMouseover();

        virtual void update(const double& dt);
        virtual void render();
        virtual void render(const glm::vec4& scissor);

        void enableTexture(const bool);
        void enableTextureCorner(const bool);
        void enableTextureEdge(const bool);
        void enableTextureCorner(const bool, unsigned int index);
        void enableTextureEdge(const bool, unsigned int index);

        void setTexture(Texture*);
        void setTexture(Handle&);

        void setTextureCorner(Texture*);
        void setTextureCorner(Handle&);
        void setTextureCorner(Texture*, unsigned int index);
        void setTextureCorner(Handle&, unsigned int index);

        void setTextureEdge(Texture*);
        void setTextureEdge(Handle&);
        void setTextureEdge(Texture*, unsigned int index);
        void setTextureEdge(Handle&, unsigned int index);

        void setTextureHighlight(Texture*);
        void setTextureHighlight(Handle&);

        void setTextureCornerHighlight(Texture*);
        void setTextureCornerHighlight(Handle&);
        void setTextureCornerHighlight(Texture*, unsigned int index);
        void setTextureCornerHighlight(Handle&, unsigned int index);

        void setTextureEdgeHighlight(Texture*);
        void setTextureEdgeHighlight(Handle&);
        void setTextureEdgeHighlight(Texture*, unsigned int index);
        void setTextureEdgeHighlight(Handle&, unsigned int index);

        Texture* getTextureCenter();
        Texture* getTextureCorner(unsigned int index = 0);
        Texture* getTextureEdge(unsigned int index = 0);

        void setDepth(const float& depth);

        void setColor(const glm::vec4& color);
        void setColorHighlight(const glm::vec4& color);

        const float& getDepth() const;

        const float  getEdgeWidth(unsigned int index = 0) const;
        const float  getEdgeHeight(unsigned int index = 0) const;
        const float  getCornerWidth(unsigned int index = 0) const;
        const float  getCornerHeight(unsigned int index = 0) const;

        const float  getBorderSize(unsigned int index) const;
        const float  getPaddingSize(unsigned int index) const;

};


#endif
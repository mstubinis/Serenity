#pragma once
#ifndef GAME_GUI_RENDER_ELEMENT_H
#define GAME_GUI_RENDER_ELEMENT_H

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class  Texture;
class  Widget;
struct Handle;
class GUIRenderElement final {
    friend class Widget;
    private:
        Widget&        m_Owner;

        glm::vec4      m_ColorBorder;
        glm::vec4      m_Color;
        glm::vec4      m_ColorHighlight;
        float          m_Depth;
        unsigned int   m_BorderSize;

        bool           m_EnableMouseover;
        bool           m_EnableCenterTexture;
        float          m_EdgeHeight;
        float          m_EdgeWidth;

        Texture*       m_TextureCenter;
        Texture*       m_TextureCorner;
        Texture*       m_TextureEdge;

        Texture*       m_TextureCenterHighlight;
        Texture*       m_TextureCornerHighlight;
        Texture*       m_TextureEdgeHighlight;

    public:
        void internal_calculate_sizes();

        GUIRenderElement(Widget& owner);
        ~GUIRenderElement();

        void enableCenterTexture(const bool = true);
        void disableCenterTexture();

        void enableMouseover(const bool = true);
        void disableMouseover();

        void update(const double& dt);
        void render();
        void render(const glm::vec4& scissor);

        void setTexture(Texture*);
        void setTextureCorner(Texture*);
        void setTextureEdge(Texture*);
        void setTexture(Handle&);
        void setTextureCorner(Handle&);
        void setTextureEdge(Handle&);


        void setTextureHighlight(Texture*);
        void setTextureCornerHighlight(Texture*);
        void setTextureEdgeHighlight(Texture*);
        void setTextureHighlight(Handle&);
        void setTextureCornerHighlight(Handle&);
        void setTextureEdgeHighlight(Handle&);


        Texture* getTextureCenter();
        Texture* getTextureCorner();
        Texture* getTextureEdge();

        void setDepth(const float& depth);

        void setColor(const glm::vec4& color);
        void setColorHighlight(const glm::vec4& color);

        const float& getDepth() const;

        const float& getEdgeWidth() const;
        const float& getEdgeHeight() const;
        const float  getCornerWidth() const;
        const float  getCornerHeight() const;

};


#endif
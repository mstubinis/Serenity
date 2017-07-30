#pragma once
#ifndef HUD_H
#define HUD_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class Font;
class Object;
class Ship;

class HUD final{
    private:
        unsigned int m_TargetIterator;
        glm::vec3 m_Color;
        Font* m_Font;
        void _renderCrosshair(Object*);

        glm::vec2 m_WarpIndicatorSize;
    public:
        HUD();
        ~HUD();

        void update(float);
        void render(bool=false);

        const glm::vec3 getColor() const;
        void setColor(glm::vec3 c);
};
#endif
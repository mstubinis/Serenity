#pragma once
#ifndef HUD_H
#define HUD_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "Engine_ResourceHandle.h"

class Font;
class Ship;
class Entity;
class HUD final{
    private:
        unsigned int m_TargetIterator;
        glm::vec3 m_Color;
        Handle m_Font;
        glm::vec2 m_WarpIndicatorSize;
    public:
        HUD();
        ~HUD();

        void update(const float& dt);
        void render();

        const glm::vec3 getColor() const;
        void setColor(glm::vec3 c);
};
#endif
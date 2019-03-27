#pragma once
#ifndef GAME_HUD_H
#define GAME_HUD_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "core/engine/resources/Engine_Resources.h"

class  Font;
class  Ship;
struct Entity;
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
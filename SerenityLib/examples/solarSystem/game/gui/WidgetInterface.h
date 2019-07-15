#pragma once
#ifndef GAME_WIDGET_INTERFACE_H
#define GAME_WIDGET_INTERFACE_H

#include "WidgetIncludes.h"
#include <glm/vec4.hpp>

class IWidget {
    public:
        virtual const float& width() const = 0;
        virtual const float& height() const = 0;

        virtual const bool isMouseOver() const = 0;

        virtual void setAlignment(const WidgetAlignment::Type& alignment) = 0;

        virtual void setColor(const float& r, const float& g, const float& b, const float& a) = 0;
        virtual void setColor(const glm::vec4& color) = 0;
        virtual void show() = 0;
        virtual void hide() = 0;

        virtual void update(const double& dt) = 0;
        virtual void render() = 0;

};



#endif
#pragma once
#ifndef GAME_SHIP_VORCHA_CLASS_H
#define GAME_SHIP_VORCHA_CLASS_H

#include "../../Ship.h"

class Vorcha final : public Ship {
    private:

    public:
        Vorcha(
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Vorcha Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Vorcha();
};

#endif
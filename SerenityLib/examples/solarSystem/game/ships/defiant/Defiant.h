#pragma once
#ifndef GAME_SHIP_DEFIANT_CLASS_H
#define GAME_SHIP_DEFIANT_CLASS_H

#include "../../Ship.h"

class Defiant final : public Ship {
    private:

    public:
        Defiant(
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Defiant Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Defiant();
};

#endif
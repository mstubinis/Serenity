#pragma once
#ifndef GAME_SHIP_BREL_H
#define GAME_SHIP_BREL_H

#include "../../Ship.h"

class Brel final : public Ship {
    private:

    public:
        Brel(
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Brel();
};

#endif
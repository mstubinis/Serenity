#pragma once
#ifndef GAME_SHIP_BORG_CUBE_H
#define GAME_SHIP_BORG_CUBE_H

#include "../../Ship.h"

class Cube final : public Ship {
    private:

    public:
        Cube(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Cube(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Borg Cube",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Cube();
};

#endif
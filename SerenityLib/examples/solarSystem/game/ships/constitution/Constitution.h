#pragma once
#ifndef GAME_SHIP_CONNIE_CLASS_H
#define GAME_SHIP_CONNIE_CLASS_H

#include "../../Ship.h"

class Constitution final : public Ship {
    private:

    public:
        Constitution(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Constitution(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Constitution Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Constitution();
};

#endif
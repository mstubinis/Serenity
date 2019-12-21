#pragma once
#ifndef GAME_SHIP_VENEREX_H
#define GAME_SHIP_VENEREX_H

#include "../../Ship.h"

class Venerex final : public Ship {
    private:

    public:
        Venerex(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Venerex(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Venerex Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Venerex();
};
#endif
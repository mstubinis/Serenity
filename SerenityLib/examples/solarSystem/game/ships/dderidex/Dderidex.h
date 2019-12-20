#pragma once
#ifndef GAME_SHIP_DDERIDEX_H
#define GAME_SHIP_DDERIDEX_H

#include "../../Ship.h"

class Dderidex final : public Ship {
    private:

    public:
        Dderidex(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Dderidex(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "D'deridex Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Dderidex();
};

#endif
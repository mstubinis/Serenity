#pragma once
#ifndef GAME_SHIP_INTREPID_CLASS_H
#define GAME_SHIP_INTREPID_CLASS_H

#include "../../Ship.h"

class Intrepid final : public Ship {
    private:

    public:
        Intrepid(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Intrepid(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Intrepid Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Intrepid();
};

#endif
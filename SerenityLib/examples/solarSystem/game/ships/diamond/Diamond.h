#pragma once
#ifndef GAME_SHIP_BORG_DIAMOND_H
#define GAME_SHIP_BORG_DIAMOND_H

#include "../../Ship.h"

class Diamond final : public Ship {
    private:

    public:
        Diamond(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Diamond(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Borg Diamond",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Diamond();
};

#endif
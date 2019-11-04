#pragma once
#ifndef GAME_SHIP_NOVA_CLASS_H
#define GAME_SHIP_NOVA_CLASS_H

#include "../../Ship.h"

class Nova final : public Ship {
    private:

    public:
        Nova(
            const AIType::Type ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Nova Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Nova();
};

#endif
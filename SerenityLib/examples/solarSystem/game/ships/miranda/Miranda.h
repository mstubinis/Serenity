#pragma once
#ifndef GAME_SHIP_MIRANDA_CLASS_H
#define GAME_SHIP_MIRANDA_CLASS_H

#include "../../Ship.h"

class Miranda final : public Ship {
    private:

    public:
        Miranda(
            Team& team,
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Miranda Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Miranda();
};

#endif
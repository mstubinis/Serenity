#pragma once
#ifndef GAME_SHIP_AKIRA_CLASS_H
#define GAME_SHIP_AKIRA_CLASS_H

#include "../../Ship.h"

class Akira final : public Ship {
    private:

    public:
        Akira(
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Akira Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Akira();
};

#endif
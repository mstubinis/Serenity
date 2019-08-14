#pragma once
#ifndef GAME_SHIP_SHRIKE_H
#define GAME_SHIP_SHRIKE_H

#include "../../Ship.h"

class Shrike final : public Ship {
    private:

    public:
        Shrike(
            Client& client,
            bool player = false,                  //Player Ship?
            const std::string& name = "Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull,
            Map* = nullptr
        );
        ~Shrike();
};

#endif
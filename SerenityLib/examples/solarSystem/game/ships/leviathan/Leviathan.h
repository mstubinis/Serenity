#pragma once
#ifndef GAME_SHIP_LEVIATHAN_CLASS_H
#define GAME_SHIP_LEVIATHAN_CLASS_H

#include "../../Ship.h"

class Leviathan final : public Ship {
    private:

    public:
        Leviathan(
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Leviathan Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Leviathan();
};

#endif
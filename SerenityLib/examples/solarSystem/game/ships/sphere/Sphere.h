#pragma once
#ifndef GAME_SHIP_BORG_SPHERE_H
#define GAME_SHIP_BORG_SPHERE_H

#include "../../Ship.h"

class Sphere final : public Ship {
    private:

    public:
        Sphere(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Sphere(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Borg Sphere",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Sphere();
};

#endif
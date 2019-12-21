#pragma once
#ifndef GAME_SHIP_NEGHVAR_H
#define GAME_SHIP_NEGHVAR_H

#include "../../Ship.h"


class Neghvar final : public Ship {
    private:

    public:
        Neghvar(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Neghvar(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Negh'var Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Neghvar();
};

#endif
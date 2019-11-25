#pragma once
#ifndef GAME_SHIP_FED_DEF_PLATFORM_H
#define GAME_SHIP_FED_DEF_PLATFORM_H

#include "../../Ship.h"

class FedDefPlatform final : public Ship {
    private:

    public:
        FedDefPlatform(
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Federation Defense Platform",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~FedDefPlatform();
};

#endif
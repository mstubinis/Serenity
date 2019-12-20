#pragma once
#ifndef GAME_SHIP_FED_STARBASE_MUSHROOM_H
#define GAME_SHIP_FED_STARBASE_MUSHROOM_H

#include "../../Ship.h"

class FedStarbaseMushroom final : public Ship {
    private:

    public:
        FedStarbaseMushroom(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        FedStarbaseMushroom(
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Federation Starbase",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        ~FedStarbaseMushroom();
};

#endif
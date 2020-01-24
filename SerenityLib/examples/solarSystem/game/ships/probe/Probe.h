#pragma once
#ifndef GAME_SHIP_BORG_PROBE_H
#define GAME_SHIP_BORG_PROBE_H

#include "../../Ship.h"

class Probe final : public Ship {
    private:

    public:
        Probe(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Probe(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Borg Probe",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Probe();
};

#endif
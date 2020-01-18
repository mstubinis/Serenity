#pragma once
#ifndef GAME_SHIP_SOVEREIGN_CLASS_H
#define GAME_SHIP_SOVEREIGN_CLASS_H

#include "../../Ship.h"

class Sovereign final : public Ship {
    private:
        enum Perks : unsigned int {
            None = 0,
            EnhancedDeflectorDish = 1 << 0, //
            RegenerativeShields = 1 << 1, //
            CorbormiteReflector = 1 << 2, //maybe
            All = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)


    public:
        Sovereign(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Sovereign(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Sovereign Class Ship",
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Sovereign();
};

#endif
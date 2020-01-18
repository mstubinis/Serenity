#pragma once
#ifndef GAME_SHIP_VENEREX_H
#define GAME_SHIP_VENEREX_H

#include "../../Ship.h"

class Venerex final : public Ship {
    private:
        enum Perks : unsigned int {
            None = 0,
            UNKNOWN_1 = 1 << 0, //
            UNKNOWN_2 = 1 << 1, //
            UNKNOWN_3 = 1 << 2, //
            All = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)

    public:
        Venerex(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Venerex(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Venerex Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Venerex();
};
#endif
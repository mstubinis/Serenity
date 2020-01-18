#pragma once
#ifndef GAME_SHIP_SHRIKE_H
#define GAME_SHIP_SHRIKE_H

#include "../../Ship.h"

class Shrike final : public Ship {
    private:
        enum Perks : unsigned int {
            None          = 0,
            ECMGenerator  = 1 << 0,
            TalShiarCloak = 1 << 1,
            RomulanSpy    = 1 << 2, //maybe
            All           = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)

    public:
        Shrike(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Shrike(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Shrike Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Shrike();
};

#endif
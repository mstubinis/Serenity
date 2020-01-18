#pragma once
#ifndef GAME_SHIP_AKIRA_CLASS_H
#define GAME_SHIP_AKIRA_CLASS_H

#include "../../Ship.h"

class Akira final : public Ship {
    private:
        enum Perks : unsigned int {
            None                = 0,
            MarkVCasings        = 1 << 0, //always active once unlocked
            ChainReactionPulsar = 1 << 1, //always active once unlocked, fireable weapon
            QuantumTorpedos     = 1 << 2, //always active once unlocked
            All                 = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)

    public:
        Akira(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Akira(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Akira Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Akira();
};

#endif
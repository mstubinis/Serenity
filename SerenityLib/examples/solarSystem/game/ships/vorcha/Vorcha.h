#pragma once
#ifndef GAME_SHIP_VORCHA_CLASS_H
#define GAME_SHIP_VORCHA_CLASS_H

#include "../../Ship.h"

class Vorcha final : public Ship {
    private:
        enum Perks : unsigned int {
            None = 0,
            DisruptorBarrage = 1 << 0, //always active once unlocked
            BerserkMode      = 1 << 1, //always active once unlocked, toggable ability
            PhasedTorpedo    = 1 << 2, //maybe
            All = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)


    public:
        Vorcha(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Vorcha(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Vorcha Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Vorcha();

        void update(const double& dt);
};

#endif
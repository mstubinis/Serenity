#pragma once
#ifndef GAME_SHIP_DDERIDEX_H
#define GAME_SHIP_DDERIDEX_H

#include "../../Ship.h"

class Dderidex final : public Ship {
    private:
        enum Perks : unsigned int {
            None                = 0,
            TachyonField        = 1 << 0, //
            ComputerHack        = 1 << 1, //
            ShieldInversionBeam = 1 << 2, //maybe
            All                 = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)
    public:
        Dderidex(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Dderidex(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "D'deridex Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Dderidex();
};

#endif
#pragma once
#ifndef GAME_SHIP_NEGHVAR_H
#define GAME_SHIP_NEGHVAR_H

#include "../../Ship.h"


class Neghvar final : public Ship {
    private:
        enum Perks : unsigned int {
            None                 = 0,
            DeathChant           = 1 << 0, //
            DisruptorCannonBlast = 1 << 1, //
            IonLance             = 1 << 2, //maybe
            All                  = 4294967295,
        };
        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)
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
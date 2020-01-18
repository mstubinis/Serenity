#pragma once
#ifndef GAME_SHIP_BREL_H
#define GAME_SHIP_BREL_H

#include "../../Ship.h"

constexpr auto BREL_WING_SPAN_MAX = 0.67771135f;


class Brel final : public Ship {
    private:
        enum Perks : unsigned int {
            None           = 0,
            SuicideRun     = 1 << 0,
            HighEnergyTurn = 1 << 1, //maybe
            CannonOverload = 1 << 2,
            All            = 4294967295,
        };


        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)


        struct BrelWingSpanState final {enum State {
            Down,
            RotatingUp,
            Up,
            RotatingDown,
        };};

        Brel::BrelWingSpanState::State m_WingState;
        glm_vec3 m_ShieldScale;
        glm_vec3 m_InitialCamera;
        float m_WingRotation;
    public:
        Brel(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Brel(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "B'rel Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Brel();

        const Brel::BrelWingSpanState::State getWingState() const;

        void updateWingSpan(const double& dt, const BrelWingSpanState::State end, const int mod);

        void foldWingsUp();
        void foldWingsDown();
        void update(const double& dt);
};

#endif
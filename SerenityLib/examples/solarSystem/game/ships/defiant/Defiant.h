#pragma once
#ifndef GAME_SHIP_DEFIANT_CLASS_H
#define GAME_SHIP_DEFIANT_CLASS_H

#include "../../Ship.h"
#include "../../ships/shipSystems/ShipSystemWeapons.h"


class Defiant final : public Ship {
    private:
        enum Perks : unsigned int {
            None                = 0,
            FrequencyModulators = 1 << 0,  //maybe (can be turned on and off)
            AblativeArmor       = 1 << 1,  //(will always be active once unlocked)
            CloakingDevice      = 1 << 2,  //(can be turned on and off)
            All                 = 4294967295,
        };


        unsigned int                                      m_Perks;         //active perks
        unsigned int                                      m_UnlockedPerks; //account unlocked perks (move them to a player class later on?)

        std::vector<ShipSystemWeapons::WeaponBeam>        m_BonusBeams;
    public:
        Defiant(
            Scene& scene,
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1)             //Scale
        );
        Defiant(
            AIType::Type& ai_type,
            Team& team,
            Client& client,
            Map& map,
            const std::string& name = "Defiant Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Defiant();

        void togglePerk1CannonsToBeams();

        void fireBeams(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship);
        void fireCannons(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship);

        bool cloak(const bool sendPacket = true);
        bool decloak(const bool sendPacket = true);

        void update(const double& dt);
};

#endif
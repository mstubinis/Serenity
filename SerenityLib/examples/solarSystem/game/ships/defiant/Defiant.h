#pragma once
#ifndef GAME_SHIP_DEFIANT_CLASS_H
#define GAME_SHIP_DEFIANT_CLASS_H

#include "../../Ship.h"
#include "../../ships/shipSystems/ShipSystemWeapons.h"


class Defiant final : public Ship {
    private:

        enum Perks : unsigned int {
            None = 0,
            DefiantPerk1BeamReplaceCannons = 1 << 0,
            DefiantPerk2AblativeArmor = 1 << 1,
            DefiantPerk3CloakingDevice = 1 << 2,
            All = 4294967295,
        };


        unsigned int                                      m_Perks;
        unsigned int                                      m_UnlockedPerks;

        std::vector<ShipSystemWeapons::WeaponBeam>        m_BonusBeams;
    public:
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
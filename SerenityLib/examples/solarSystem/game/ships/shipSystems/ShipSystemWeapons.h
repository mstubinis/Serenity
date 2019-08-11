#pragma once
#ifndef GAME_SHIP_SYSTEM_WEAPONS_H
#define GAME_SHIP_SYSTEM_WEAPONS_H

#include "ShipSystemBaseClass.h"
#include <vector>
#include <glm/vec3.hpp>

struct PrimaryWeaponCannon {
    Ship& ship;
    glm::vec3 position; //relative to the ship's model center
    glm::vec3 forward;
    float arc;
    PrimaryWeaponCannon(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc);
    virtual bool fire();
    virtual void update(const double& dt);
};

struct PrimaryWeaponBeam {
    Ship& ship;
    glm::vec3 position; //relative to the ship's model center
    glm::vec3 forward;
    float arc;
    PrimaryWeaponBeam(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc);
    virtual bool fire();
    virtual void update(const double& dt);
};

struct SecondaryWeaponTorpedo {
    Ship& ship;
    glm::vec3 position; //relative to the ship's model center
    glm::vec3 forward;
    float arc;
    SecondaryWeaponTorpedo(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc);
    virtual bool fire();
    virtual void update(const double& dt);
};


class ShipSystemWeapons final : public ShipSystem {
    friend class Ship;
    private:
        std::vector<PrimaryWeaponCannon*>      m_PrimaryWeaponsCannons;
        std::vector<PrimaryWeaponBeam*>        m_PrimaryWeaponsBeams;
        std::vector<SecondaryWeaponTorpedo*>   m_SecondaryWeaponsTorpedos;
    public:
        ShipSystemWeapons(Ship&);
        ~ShipSystemWeapons();

        void addPrimaryWeaponBeam(PrimaryWeaponBeam&);
        void addPrimaryWeaponCannon(PrimaryWeaponCannon&);
        void addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo&);

        void update(const double& dt);
};

#endif
#pragma once
#ifndef GAME_SHIP_SYSTEM_CLOAKING_DEVICE_H
#define GAME_SHIP_SYSTEM_CLOAKING_DEVICE_H

#include "ShipSystemBaseClass.h"

class ComponentModel;
class ShipSystemCloakingDevice final : public ShipSystem {
    friend class Ship;
    private:
        bool  m_Active;
        float m_CloakTimer;
    public:
        ShipSystemCloakingDevice(Ship&);
        ~ShipSystemCloakingDevice();

        const bool  isCloakActive() const;
        const float getCloakTimer() const;

        void update(const double& dt);
        bool cloak(ComponentModel&, bool sendPacket = true);
        bool decloak(ComponentModel&, bool sendPacket = true);
};

#endif
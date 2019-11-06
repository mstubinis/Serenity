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
        float m_CloakSpeed;
        float m_MaxAlphaWhileCloaked;
    public:
        ShipSystemCloakingDevice(Ship&, const float cloakTimer = 1.0f, const float maxAlphaWhileCloaked = 0.2f);
        ~ShipSystemCloakingDevice();

        const bool  isCloakActive() const;
        const float getCloakTimer() const;
        const float getMaxAlphaWhileCloaked() const;

        void update(const double& dt);
        const bool cloak(ComponentModel&, bool sendPacket = true);
        const bool decloak(ComponentModel&, bool sendPacket = true);
};

#endif
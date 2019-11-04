#pragma once
#ifndef GAME_SHIP_SYSTEM_SENSORS_H
#define GAME_SHIP_SYSTEM_SENSORS_H

#include "ShipSystemBaseClass.h"
#include <ecs/Entity.h>
#include <core/engine/scene/Viewport.h>

class Map;
class Camera;
class Ship;

struct DetectedShip final {
    Ship*   ship;
    decimal distanceAway2;
    DetectedShip() {
        ship = nullptr;
        distanceAway2 = static_cast<decimal>(-1.0);
    }
};

class ShipSystemSensors final : public ShipSystem {
    friend class Ship;
    private:

        struct Detection final {
            bool     valid;
            decimal  distanceSquared;
        };

        EntityWrapper*             m_Target;
        Map&                       m_Map;
        decimal                    m_RadarRange;

        std::vector<DetectedShip>  m_DetectedEnemyShips;
        std::vector<DetectedShip>  m_DetectedShips;
        std::vector<DetectedShip>  m_DetectedAlliedShips;
        std::vector<DetectedShip>  m_DetectedNeutralShips;
    public:
        ShipSystemSensors(Ship&, Map&, const decimal& range = static_cast<decimal>(1000.0)); //100km
        ~ShipSystemSensors();

        const ShipSystemSensors::Detection validateDetection(Ship& othership, const glm_vec3& thisShipPos);
        const decimal& getRadarRange() const;

        DetectedShip getClosestAlliedShip();
        DetectedShip getClosestNeutralShip();
        DetectedShip getClosestEnemyShip();
        DetectedShip getClosestEnemyCloakedShip();
        DetectedShip getClosestShip();

        std::vector<DetectedShip>&   getEnemyShips();
        std::vector<DetectedShip>&   getShips();
        std::vector<DetectedShip>&   getAlliedShips();
        std::vector<DetectedShip>&   getNeutralShips();

        EntityWrapper* getTarget();
        void setTarget(EntityWrapper* entityWrapper, const bool sendPacket);
        void setTarget(const std::string& name, const bool sendPacket);

        void update(const double& dt);
        void render();
};

#endif
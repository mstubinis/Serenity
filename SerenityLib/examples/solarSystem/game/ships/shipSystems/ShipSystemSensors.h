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
        EntityWrapper*      m_Target;
        Map&                m_Map;
        //Camera*             m_Camera;
        //Entity              m_RadarRingEntity;
        decimal             m_RadarRange;
        //glm::vec4           m_Viewport;
        //Viewport*           m_ViewportObject;

        std::vector<Ship*>  m_DetectedEnemyShips;
        std::vector<Ship*>  m_DetectedShips;
        std::vector<Ship*>  m_DetectedAlliedShips;
        std::vector<Ship*>  m_DetectedNeutralShips;
    public:
        ShipSystemSensors(Ship&, Map&, const decimal& range = static_cast<decimal>(1000.0)); //100km
        ~ShipSystemSensors();

        const bool validateDetection(Ship& othership, const glm_vec3& thisShipPos);
        const decimal& getRadarRange() const;

        DetectedShip getClosestAlliedShip();
        DetectedShip getClosestNeutralShip();
        DetectedShip getClosestEnemyShip();
        DetectedShip getClosestShip();

        std::vector<Ship*>&   getEnemyShips();
        std::vector<Ship*>&   getShips();
        std::vector<Ship*>&   getAlliedShips();
        std::vector<Ship*>&   getNeutralShips();

        //const Entity& radarRingEntity() const;
        //const Entity& radarCameraEntity() const;

        //void onResize(const uint& width, const uint& height);

        EntityWrapper* getTarget();
        void setTarget(EntityWrapper* entityWrapper, const bool sendPacket);
        void setTarget(const std::string& name, const bool sendPacket);

        void update(const double& dt);
        void render();
};

#endif
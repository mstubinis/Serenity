#pragma once
#ifndef GAME_SHIP_SYSTEM_SENSORS_H
#define GAME_SHIP_SYSTEM_SENSORS_H

#include "ShipSystemBaseClass.h"
#include <ecs/EntityWrapper.h>
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
        struct AntiCloakDetection final {
            Ship* ship;
            double detection_timer_max;
            double detection_timer_current;
        };

        bool                       m_IsPingingForShips;
        double                     m_IsPingingForShipsTimer;

        unsigned int               m_CurrentCycleEnemyIndex;
        unsigned int               m_CurrentCycleFriendlyIndex;
        unsigned int               m_CurrentCycleNeutralIndex;

        double                     m_AntiCloakScanTimerMax;
        double                     m_AntiCloakScanTimer;
        bool                       m_AntiCloakScanActive;

        EntityWrapper*             m_Target;
        Map&                       m_Map;
        decimal                    m_RadarRange;

        std::vector<AntiCloakDetection>  m_DetectedAntiCloakedShips;
        std::vector<DetectedShip>        m_DetectedEnemyShips;
        std::vector<DetectedShip>        m_DetectedShips;
        std::vector<DetectedShip>        m_DetectedAlliedShips;
        std::vector<DetectedShip>        m_DetectedNeutralShips;

        void internal_update_clear_target_automatically_if_applicable(const double& dt);
        void internal_update_anti_cloak_scan(const double& dt);
        void internal_update_anti_cloak_scan_detected_ships(const double& dt);
        void internal_update_populate_detected_ships(const double& dt);
    public:
        //                                                                         //100km
        ShipSystemSensors(Ship&, Map&, const decimal& range = static_cast<decimal>(1000.0), const double AntiCloakScanTimerMax = 5.0);
        ~ShipSystemSensors();

        ShipSystemSensors(const ShipSystemSensors& other)                = delete;
        ShipSystemSensors& operator=(const ShipSystemSensors& other)     = delete;
        ShipSystemSensors(ShipSystemSensors&& other) noexcept            = delete;
        ShipSystemSensors& operator=(ShipSystemSensors&& other) noexcept = delete;

        const ShipSystemSensors::Detection validateDetection(Ship& othership, const glm_vec3& thisShipPos);
        const decimal& getRadarRange() const;

        const bool isShipDetectedByAntiCloak(Ship* ship);
        const bool toggleAntiCloakScan(const bool sendPacket = false);
        const bool disableAntiCloakScan(const bool sendPacket = false);
        void sendAntiCloakScanStatusPacket();

        const double& getAntiCloakingScanTimer() const;
        const double& getAntiCloakingScanTimerMax() const;

        const bool& isAntiCloakScanActive() const;

        DetectedShip getClosestAlliedShip();
        DetectedShip getClosestNeutralShip();
        DetectedShip getClosestEnemyShip();
        DetectedShip getClosestEnemyCloakedShip();
        DetectedShip getClosestShip();

        std::vector<AntiCloakDetection>&  getAntiCloakDetectedShips();
        std::vector<DetectedShip>&        getEnemyShips();
        std::vector<DetectedShip>&        getShips();
        std::vector<DetectedShip>&        getAlliedShips();
        std::vector<DetectedShip>&        getNeutralShips();

        EntityWrapper* getTarget();
        void setTarget(EntityWrapper* entityWrapper, const bool sendPacket);
        void setTarget(const std::string& name, const bool sendPacket);

        void update(const double& dt);
        void render();
};

#endif
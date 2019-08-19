#pragma once
#ifndef GAME_SHIP_SYSTEM_SENSORS_H
#define GAME_SHIP_SYSTEM_SENSORS_H

#include "ShipSystemBaseClass.h"
#include <ecs/Entity.h>
#include <core/engine/scene/Viewport.h>

class Map;
class Camera;
class ShipSystemSensors final : public ShipSystem {
    friend class Ship;
    private:
        Map&        m_Map;
        Camera*     m_Camera;
        Entity      m_RadarRingEntity;
        float       m_RadarRange;
        glm::vec4   m_Viewport;
        Viewport*   m_ViewportObject;
    public:
        ShipSystemSensors(Ship&, Map&, const float& range = 100.0f); //10km
        ~ShipSystemSensors();

        const Entity& radarRingEntity() const;
        const Entity& radarCameraEntity() const;

        void onResize(const uint& width, const uint& height);
        void update(const double& dt);
        void render();
};

#endif
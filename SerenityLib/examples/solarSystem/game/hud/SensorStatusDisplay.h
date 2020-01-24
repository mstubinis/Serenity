#pragma once
#ifndef GAME_SENSOR_STATUS_DISPLAY_H
#define GAME_SENSOR_STATUS_DISPLAY_H

#include <ecs/Entity.h>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class  Ship;
class  Map;
class  ShipSystemSensors;
class  Camera;
class  Viewport;
class  HUD;
class SensorStatusDisplay final {
    private:
        HUD&                m_HUD;
        Map&                m_Map;
        Ship*               m_Ship;
        ShipSystemSensors*  m_Sensors;

        Camera*             m_Camera;
        Entity              m_RadarRingEntity;
        glm::vec4           m_Viewport;
        Viewport*           m_ViewportObject;


        glm::vec2        m_Position;
        glm::vec2        m_Size;
        glm::vec4        m_Color;
        Alignment::Type  m_Alignment;

    public:
        SensorStatusDisplay(HUD&, Map&, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, const Alignment::Type& alignment);
        ~SensorStatusDisplay();

        SensorStatusDisplay(const SensorStatusDisplay& other)                = delete;
        SensorStatusDisplay& operator=(const SensorStatusDisplay& other)     = delete;
        SensorStatusDisplay(SensorStatusDisplay&& other) noexcept            = delete;
        SensorStatusDisplay& operator=(SensorStatusDisplay&& other) noexcept = delete;

        void setShip(Ship*);

        const Entity& radarRingEntity() const;
        const Entity& radarCameraEntity() const;
        const glm::vec2& size() const;

        void setPosition(const float x, const float y);
        void onResize(const unsigned int& width, const unsigned int& height);

        void update(const double& dt);
        void render();

};

#endif
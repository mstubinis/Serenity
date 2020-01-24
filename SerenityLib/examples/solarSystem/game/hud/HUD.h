#pragma once
#ifndef GAME_SHIP_HUD_H
#define GAME_SHIP_HUD_H

class  Ship;
class  Planet;
class  EntityWrapper;
struct Entity;
class  Map;
class  SensorStatusDisplay;
class  ShipStatusDisplay;
class  TargetRedicule;
class  Font;

class HUD final {
    private:
        Map& m_Map;
        Font& m_Font;
        SensorStatusDisplay*  m_SensorDisplay;
        ShipStatusDisplay*    m_ShipStatusDisplay;
        ShipStatusDisplay*    m_ShipTargetStatusDisplay;
        TargetRedicule*       m_TargetRedicule;
        bool m_Shown;
          
    public:
        HUD(Map&, Font&);
        ~HUD();

        HUD(const HUD& other)                = delete;
        HUD& operator=(const HUD& other)     = delete;
        HUD(HUD&& other) noexcept            = delete;
        HUD& operator=(HUD&& other) noexcept = delete;

        SensorStatusDisplay& getSensorDisplay();
        ShipStatusDisplay&   getShipStatusDisplay();
        ShipStatusDisplay&   getShipTargetStatusDisplay();

        Font& getFont();
        Map& getMap();

        void setTarget(Planet*);
        void setTarget(Ship*);
        void setTarget(EntityWrapper*);
        void setTarget(Entity&);

        const bool isHidden() const;

        void show(const bool shown = true);
        void hide();
        void toggle();

        void onResize(const unsigned int& width, const unsigned int& height);

        void update(const double& dt);
        void render();
};

#endif
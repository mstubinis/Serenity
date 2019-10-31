#pragma once
#ifndef GAME_SHIP_HUD_H
#define GAME_SHIP_HUD_H

class  Ship;
class  Planet;
struct EntityWrapper;
struct Entity;
class  Map;
class  SensorStatusDisplay;
class  ShipStatusDisplay;

class HUD final {
    private:
        Map& m_Map;

        SensorStatusDisplay*  m_SensorDisplay;
        ShipStatusDisplay*    m_ShipStatusDisplay;
        ShipStatusDisplay*    m_ShipTargetStatusDisplay;
          
    public:
        HUD(Map&);
        ~HUD();

        SensorStatusDisplay& getSensorDisplay();
        ShipStatusDisplay&   getShipStatusDisplay();
        ShipStatusDisplay&   getShipTargetStatusDisplay();

        void setTarget(Planet*);
        void setTarget(Ship*);
        void setTarget(EntityWrapper*);
        void setTarget(Entity&);

        void onResize(const unsigned int& width, const unsigned int& height);

        void update(const double& dt);
        void render();
};

#endif
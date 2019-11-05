#pragma once
#ifndef GAME_FIRE_AT_WILL_H
#define GAME_FIRE_AT_WILL_H

class Ship;
class Map;
class ShipSystemSensors;
class ShipSystemWeapons;

#include <random>

class FireAtWill final {
    private:
        std::mt19937         m_random_device;

        Ship&                m_Ship;
        Map&                 m_Map;
        ShipSystemSensors&   m_Sensors;
        ShipSystemWeapons&   m_Weapons;
        bool                 m_Activated;
        double               m_TimerBeam;
        double               m_TimerCannon;
        double               m_TimerTorpedo;

        void internal_reset_timers();
        void internal_execute_beams();
        void internal_execute_cannons();
        void internal_execute_torpedos();
    public:
        FireAtWill(Ship&, Map&, ShipSystemSensors&, ShipSystemWeapons&);
        ~FireAtWill();

        void activate();
        void deactivate();
        void toggle();
        void update(const double& dt);
};


#endif
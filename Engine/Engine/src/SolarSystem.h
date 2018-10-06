#pragma once
#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "core/Scene.h"

class Star;
class Ship;
class GameCamera;
class Planet;

class SolarSystem: public Scene{
    private:
        std::unordered_map<std::string,Planet*> m_Planets;

        Ship* player;
        float giGlobal;
        float giDiffuse;
        float giSpecular;

        void _loadFromFile(std::string);
    public:
        SolarSystem(std::string name, std::string file);
        virtual ~SolarSystem();

        virtual void onEvent(const Event& e);
        virtual void update(const float& dt);

        Ship* getPlayer(){ return player; }
        void setPlayer(Ship* p){ player = p; }

        std::unordered_map<std::string,Planet*>& getPlanets() { return m_Planets; }
};
#endif
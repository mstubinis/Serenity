#pragma once
#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Scene.h"

class Star;
class Object;
class Ship;
class GameCamera;
class Planet;

class SolarSystem: public Scene{
    private:
        std::unordered_map<std::string,Planet*> m_Planets;
        std::unordered_map<std::string,Planet*> m_Moons;
        std::unordered_map<std::string,Star*> m_Stars;

        Ship* player;

        void _loadRandomly();
        void _loadFromFile(std::string);
    public:
        SolarSystem(std::string name, std::string file);
        virtual ~SolarSystem();

        virtual void update(float);

        Ship* getPlayer(){ return player; }
        void setPlayer(Ship* p){ player = p; }

        std::unordered_map<std::string,Planet*>& getPlanets() { return m_Planets; }
        std::unordered_map<std::string,Planet*>& getMoons() { return m_Moons; }
        std::unordered_map<std::string,Star*>& getStars() { return m_Stars; }
};
#endif
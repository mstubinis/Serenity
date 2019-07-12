#pragma once
#ifndef GAME_SOLARSYSTEM_H
#define GAME_SOLARSYSTEM_H

#include <core/Scene.h>

class Star;
class Ship;
class Planet;
class GameObject;
class EntityWrapper;
class GameSkybox;

class SolarSystem: public Scene{
    private:
        std::unordered_map<std::string, Planet*> m_Planets;
        std::unordered_map<std::string, Ship*>   m_Ships;

        Ship* player;

        void loadFromFile(const std::string& file);
    public:
        std::vector<EntityWrapper*> m_Objects;

        SolarSystem(const std::string& name, const std::string& file);
        virtual ~SolarSystem();

        virtual void update(const double& dt);

        Ship* getPlayer(){ return player; }
        void setPlayer(Ship* p){ player = p; }

        std::unordered_map<std::string, Planet*>& getPlanets() { return m_Planets; }
        std::unordered_map<std::string, Ship*>& getShips() { return m_Ships; }
};
#endif
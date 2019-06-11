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
        std::unordered_map<std::string,Planet*> m_Planets;

		GameSkybox* box1;
		GameSkybox* box2;
		GameSkybox* box3;
		GameSkybox* box4;

        Ship* player;
        float giGlobal;
        float giDiffuse;
        float giSpecular;

        void _loadFromFile(std::string);
    public:
        std::vector<EntityWrapper*> m_Objects;

        SolarSystem(std::string name, std::string file);
        virtual ~SolarSystem();

        virtual void onEvent(const Event& e);
        virtual void update(const double& dt);

        Ship* getPlayer(){ return player; }
        void setPlayer(Ship* p){ player = p; }

        std::unordered_map<std::string,Planet*>& getPlanets() { return m_Planets; }
};
#endif
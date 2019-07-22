#pragma once
#ifndef GAME_SOLARSYSTEM_H
#define GAME_SOLARSYSTEM_H

#include <core/engine/scene/Scene.h>
#include <unordered_map>

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
        std::string                              m_Filename;
        Ship*                                    m_Player;
        EntityWrapper*                           m_AnchorPoint; //using this to sync warp displaced positions in multiplayer

        void loadFromFile(const std::string& file);
    public:
        std::vector<EntityWrapper*> m_Objects;

        SolarSystem(const std::string& name, const std::string& file);
        virtual ~SolarSystem();

        virtual void update(const double& dt);

        Ship* getPlayer() { return m_Player; }
        void setPlayer(Ship* p){ m_Player = p; }

        std::vector<std::string> allowedShips();
        std::string allowedShipsSingleString();

        std::unordered_map<std::string, Planet*>& getPlanets() { return m_Planets; }
        std::unordered_map<std::string, Ship*>& getShips() { return m_Ships; }
};
#endif
#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Scene.h"

class Star;
class Object;
class Ship;
class GameCamera;
class Planet;
class Station;
class Lagrange;

class SolarSystem: public Scene{
    private:
        std::map<std::string,Station*> m_Stations;
        std::map<std::string,Lagrange*> m_LagrangePoints;

        std::map<std::string,Planet*> m_Planets;
        std::map<std::string,Planet*> m_Moons;
        std::map<std::string,Star*> m_Stars;

        Ship* player;
        GameCamera* playerCamera;

        void _loadRandomly();
        void _loadFromFile(std::string);
		void _loadTest();
    public:
        SolarSystem(std::string name, std::string file, bool test = false);
        virtual ~SolarSystem();

        virtual void update(float);

        Ship* getPlayer(){ return player; }
        void setPlayer(Ship* p){ player = p; }
        GameCamera* getPlayerCamera(){ return playerCamera; }
        void setPlayerCamera(GameCamera* c){ playerCamera = c; }
        std::map<std::string,Station*>& getStations() { return m_Stations; }
        std::map<std::string,Lagrange*>& getLagrangePoints() { return m_LagrangePoints; }

        std::map<std::string,Planet*>& getPlanets() { return m_Planets; }
        std::map<std::string,Planet*>& getMoons() { return m_Moons; }
        std::map<std::string,Star*>& getStars() { return m_Stars; }
};
#endif
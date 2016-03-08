#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Scene.h"
#include <unordered_map>
#include <map>

class Star;
class Object;
class Ship;
class GameCamera;
class Planet;
class Station;
class Lagrange;

class SolarSystem final: public Scene{
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
	public:
		SolarSystem(std::string name, std::string file);
		~SolarSystem();

		void update(float);
		void render();

		Ship* getPlayer(){ return player; }
		GameCamera* getPlayerCamera(){ return playerCamera; }
		std::map<std::string,Station*>& getStations() { return m_Stations; }
		std::map<std::string,Lagrange*>& getLagrangePoints() { return m_LagrangePoints; }

		std::map<std::string,Planet*>& getPlanets() { return m_Planets; }
		std::map<std::string,Planet*>& getMoons() { return m_Moons; }
		std::map<std::string,Star*>& getStars() { return m_Stars; }
};
#endif
#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Scene.h"

class Star;
class Object;
class PlayerShip;
class GameCamera;
class Planet;

class SolarSystem: public Scene{
	private:
		std::vector<Planet*> m_Planets;
		std::vector<Planet*> m_Moons;
		std::vector<Star*> m_Stars;

		PlayerShip* player;
		GameCamera* playerCamera;

		void _loadRandomly();
		void _loadFromFile(std::string);
	public:
		SolarSystem(std::string name, std::string file);
		~SolarSystem();

		void update(float);
		void render();

		PlayerShip* getPlayer(){ return player; }
		std::vector<Planet*>& getPlanets() { return m_Planets; }
		std::vector<Planet*>& getMoons() { return m_Moons; }
		std::vector<Star*>& getStars() { return m_Stars; }
};
#endif
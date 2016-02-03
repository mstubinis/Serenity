#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Scene.h"
#include <unordered_map>

class Star;
class Object;
class PlayerShip;
class GameCamera;
class Planet;

class SolarSystem final: public Scene{
	private:
		std::unordered_map<std::string,Planet*> m_Planets;
		std::unordered_map<std::string,Planet*> m_Moons;
		std::unordered_map<std::string,Star*> m_Stars;

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
		std::unordered_map<std::string,Planet*>& getPlanets() { return m_Planets; }
		std::unordered_map<std::string,Planet*>& getMoons() { return m_Moons; }
		std::unordered_map<std::string,Star*>& getStars() { return m_Stars; }
};
#endif
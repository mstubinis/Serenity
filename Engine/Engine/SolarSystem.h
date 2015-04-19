#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Scene.h"

class Object;
class ObjectDynamic;
class GameCamera;
class Planet;

class SolarSystem: public Scene{
	private:
		std::unordered_map<std::string,Planet*> m_Planets;
		std::unordered_map<std::string,Planet*> m_Moons;
		std::unordered_map<std::string,Planet*> m_Stars;

		ObjectDynamic* player;
		GameCamera* playerCamera;

		void _loadTestSystem();
		void _loadRandomly();
		void _loadFromFile(std::string);
	public:
		SolarSystem(std::string name, std::string file);
		~SolarSystem();

		void update(float);
		void render();
};
#endif
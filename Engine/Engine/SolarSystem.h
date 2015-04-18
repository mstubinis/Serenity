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

		void LoadTestSystem();
		void LoadRandomly();
		void LoadFromFile(std::string);
	public:
		SolarSystem(std::string name, std::string file);
		~SolarSystem();

		void Update(float);
		void Render();
};
#endif
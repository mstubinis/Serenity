#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "Engine_Resources.h"
#include "Planet.h"
#include "GameCamera.h"
#include "Ship.h"

class SolarSystem{
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
		SolarSystem(std::string = "");
		~SolarSystem();

		void Update(float);
		void Render();
};
#endif
#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>

class SolarSystem;

class Game{
	private:
		std::unordered_map<std::string, SolarSystem*> m_SolarSystems;
	public:
		Game();
		~Game();

		void initResources();
		void initLogic();

		void update(float);
};
#endif
#ifndef GAME_H
#define GAME_H

#include <unordered_map>
#include "SolarSystem.h"

class Game{
	private:
		std::unordered_map<std::string, SolarSystem*> m_SolarSystems;
		SolarSystem* m_CurrentSolarSystem;
	public:
		Game();
		~Game();

		void Init_Resources();
		void Init_Logic();

		void Update(float);
};
#endif
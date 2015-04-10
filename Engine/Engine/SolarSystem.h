#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

class Object;
class ObjectDynamic;
class GameCamera;
class Planet;

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
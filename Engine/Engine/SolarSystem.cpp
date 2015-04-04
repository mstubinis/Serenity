#include "SolarSystem.h"

SolarSystem::SolarSystem(std::string file)
{
	if(file == ""){
		SolarSystem::LoadRandomly();
	}
	else{
		SolarSystem::LoadFromFile(file);
	}
}
SolarSystem::~SolarSystem()
{
}
void SolarSystem::LoadFromFile(std::string)
{
}
void SolarSystem::LoadRandomly()
{
	//solar systems are normally mono - trinary. Load 1 - 3 stars

	//Then load planets. Generally the more stars, the more planets

	//Then load moons. Generally the number of moons depends on the type of planet. Gas giants have more moons than rocky planets.
}
void SolarSystem::Update(float dt)
{
}
void SolarSystem::Render()
{
}
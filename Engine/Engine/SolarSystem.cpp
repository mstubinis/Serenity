#include "Engine_Events.h"
#include "SolarSystem.h"
#include "Engine_Resources.h"
#include "Planet.h"
#include "GameCamera.h"
#include "Ship.h"
#include "Skybox.h"

using namespace Engine;
using namespace Engine::Events;

SolarSystem::SolarSystem(std::string file){
	if(file == ""){
		//SolarSystem::LoadRandomly();
		SolarSystem::LoadTestSystem();
	}
	else{
		SolarSystem::LoadFromFile(file);
	}
}
SolarSystem::~SolarSystem()
{
}
void SolarSystem::LoadTestSystem(){
	playerCamera = new GameCamera(45,Resources::getWindow()->getSize().x/(float)Resources::getWindow()->getSize().y,0.1f,9000000000.0f);
	Resources::setActiveCamera(playerCamera);

	new Skybox("Basic");
	player = new PlayerShip("Defiant","Defiant","USS Defiant",glm::vec3(0,0,0));
	Ship* other = new Ship("Defiant","Defiant","USS Valiant",glm::vec3(2,0,0));
	for(int i = 0; i < 1; i++){
		Planet* p = new Planet("Earth",PLANET_TYPE_ROCKY,glm::vec3(-83710,0,-(i * 130000)),63710);
	}
	playerCamera->Follow(player);

	Star* sun = new Star(glm::vec3(1,0.6f,0),glm::vec3(1,1,1),glm::vec3(0,0,-279580000),6958000,"Sun");
}
void SolarSystem::LoadFromFile(std::string)
{
}
void SolarSystem::LoadRandomly(){
	//solar systems are normally mono - trinary. Load 1 - 3 stars

	//Then load planets. Generally the more stars, the more planets

	//Then load moons. Generally the number of moons depends on the type of planet. Gas giants have more moons than rocky planets.
}
void SolarSystem::Update(float dt){
	if(Keyboard::IsKeyDown("esc"))
		exit(EXIT_SUCCESS);
}
void SolarSystem::Render()
{
}
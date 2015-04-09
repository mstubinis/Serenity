#include "Engine_Events.h"
#include "SolarSystem.h"

using namespace Engine::Events;

SolarSystem::SolarSystem(std::string file)
{
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
	playerCamera = new GameCamera(45,Window->getSize().x/(float)Window->getSize().y,0.5f,50000000.0f);
	Resources->Set_Active_Camera(playerCamera);

	new Skybox("Basic");
	new Ship("Voyager","Voyager","USS Voyager",glm::vec3(4,0,0));

	player = new PlayerShip("Defiant","Defiant","USS Defiant",glm::vec3(0,0,0));
	
	
	Planet* p = new Planet("Earth",PLANET_TYPE_ROCKY,glm::vec3(-83710,0,0));
	p->Set_Scale(63710,63710,63710);
	playerCamera->Follow(player);

	Star* sun = new Star(glm::vec3(1,0.6f,0),glm::vec3(1,1,1),glm::vec3(0,0,-169580000),6958000,"Sun");
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
	if(Keyboard::IsKeyDown("esc")) exit(EXIT_SUCCESS);

	if(Keyboard::IsKeyDown("w") == true)
		player->Apply_Force(0,0,-1);
	if(Keyboard::IsKeyDown("s") == true)
		player->Apply_Force(0,0,1);
	if(Keyboard::IsKeyDown("a") == true)
		player->Apply_Force(-1,0,0);
	if(Keyboard::IsKeyDown("r") == true)
		player->Apply_Force(0,1,0);
	if(Keyboard::IsKeyDown("f") == true)
		player->Apply_Force(0,-1,0);
	if(Keyboard::IsKeyDown("d") == true)
		player->Apply_Force(1,0,0);
	if(Keyboard::IsKeyDown("q") == true)
		player->Apply_Torque(0,0,1);
	if(Keyboard::IsKeyDown("e") == true)
		player->Apply_Torque(0,0,-1);

	if(Keyboard::IsKeyDown("f1") == true)
		playerCamera->Follow(player);
	if(Keyboard::IsKeyDown("f2") == true)
		playerCamera->Orbit(player);

	if(playerCamera->State() != CAMERA_STATE_ORBIT){
		player->Apply_Torque(-Mouse_Difference.y*0.005f,-Mouse_Difference.x*0.005f,0);
	}
}
void SolarSystem::Render()
{
}
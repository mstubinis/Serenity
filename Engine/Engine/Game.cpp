#include "Game.h"
#include "Engine.h"
#include "Engine_Events.h"

using namespace Engine;
using namespace Engine::Events;

Game::Game(){}
Game::~Game(){}
void Game::Init_Logic(){
	playerCamera = new GameCamera(45,Window->getSize().x/(float)Window->getSize().y,0.1f,1000000.0f);
	Resources->Set_Active_Camera(playerCamera);

	new Skybox("Basic");
	new ObjectDynamic("Voyager","Voyager",glm::vec3(4,0,0));

	player = new ObjectDynamic("Defiant","Defiant",glm::vec3(0,0,0));
	
	
	Planet* p = new Planet("Earth",PLANET_TYPE_ROCKY,glm::vec3(-83710,0,0));
	p->Set_Scale(63710,63710,63710);
	playerCamera->Follow(player);

	SunLight* sun = new SunLight(glm::vec3(0,0,-800000));
}
void Game::Init_Resources()
{
}
void Game::Update(float dt){
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
#include "Game.h"
#include "Engine.h"
#include "Engine_Events.h"

using namespace Engine;
using namespace Engine::Events;

Game::Game(){}
Game::~Game()
{
}
void Game::Init_Logic(){

	new Skybox("Basic");
	player = new ObjectDynamic("Defiant","Defiant",glm::vec3(0,0,0));
	playerCamera = new GameCamera(60,float(Window->getSize().x/Window->getSize().y),0.01f,1000.0f);
	Resources->Set_Active_Camera(playerCamera);
	new ObjectDynamic("Voyager","Voyager",glm::vec3(2,0,0));
	//new Planet(Resources->Get_Material("Earth"),glm::vec3(-2,0,0));

	playerCamera->Follow(player);

	SunLight* sun = new SunLight(glm::vec3(0,0,-8));
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

	if(Keyboard::IsKeyDownOnce("f1") == true)
		playerCamera->Follow(player);
	if(Keyboard::IsKeyDownOnce("f2") == true)
		playerCamera->Orbit(player);

	if(playerCamera->State() != CAMERA_STATE_ORBIT){
		player->Apply_Torque(-Mouse_Difference.y*0.005f,-Mouse_Difference.x*0.005f,0);
	}
}
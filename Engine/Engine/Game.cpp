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

	new Skybox("Skybox1");
	player = new ObjectDynamic("Voyager","Voyager",glm::vec3(0,0,0));
	new ObjectDynamic("Voyager","Voyager",glm::vec3(4,0,0));
	//new Planet(Resources->Get_Material("Earth"),glm::vec3(-2,0,0));

	Resources->Current_Camera()->Set_Position(0,glm::length(player->Radius())*0.2f,glm::length(player->Radius())*1.5f);
	Resources->Current_Camera()->LookAt(player,true);
	player->Add_Child(Resources->Current_Camera());

	PointLight* pL = new PointLight(glm::vec3(0,0,-8));
}
void Game::Init_Resources()
{
}
void Game::Update(float dt){
	if(Keyboard::IsKeyDown("esc")) exit(EXIT_SUCCESS);

	if(Keyboard::IsKeyDown("w") == true)
		player->Apply_Force(0,0,1);
	if(Keyboard::IsKeyDown("s") == true)
		player->Apply_Force(0,0,-1);
	if(Keyboard::IsKeyDown("a") == true)
		player->Apply_Force(-1,0,0);
	if(Keyboard::IsKeyDown("d") == true)
		player->Apply_Force(1,0,0);
	if(Keyboard::IsKeyDown("q") == true){
		player->Rotate(0,0,1);
	}
	if(Keyboard::IsKeyDown("e") == true){
		player->Rotate(0,0,-1);
	}

	if(Keyboard::IsKeyDown("f1") == true)
		Resources->Objects.at(4)->Scale(5,5,5);
	if(Keyboard::IsKeyDown("f2") == true)
		Resources->Objects.at(4)->Scale(-5,-5,-5);

	player->Rotate(Mouse_Difference.y*0.005f,Mouse_Difference.x*0.005f,0);
}
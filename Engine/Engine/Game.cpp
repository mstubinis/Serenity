#include "Game.h"
#include "Engine.h"
#include "Engine_Events.h"
#include "Planet.h"
using namespace Engine;
using namespace Engine::Events;

Game::Game(){}
Game::~Game(){}
void Game::Init_Logic(){

	new Skybox("Skybox1");
	new ObjectDynamic("Voyager","Voyager",glm::vec3(0,0,-6));
	new ObjectDynamic("Voyager","Voyager",glm::vec3(3,0,-6));
	//new Planet(Resources->Get_Material("Earth"),glm::vec3(-2,0,0));

	PointLight* pL = new PointLight(glm::vec3(0,0,-8));

	/*
	DirectionalLight* dL = new DirectionalLight();

	dL->Color = glm::vec3(1,1,1);
	dL->AmbientIntensity = 0.1f;
	dL->DiffuseIntensity = 0.4f;
	dL->Direction = glm::vec3(-0.5,0.8,0.3);
	Resources->Lights_Directional.push_back(dL);
	*/
}
void Game::Init_Resources()
{
}
void Game::Update(float dt){
	if(Keyboard::IsKeyDown("esc"))
		exit(EXIT_SUCCESS);

	if(Keyboard::IsKeyDown("w") == true)
		Resources->Current_Camera()->Translate(0,0,10.0f);
	if(Keyboard::IsKeyDown("s") == true)
		Resources->Current_Camera()->Translate(0,0,-10.0f);
	if(Keyboard::IsKeyDown("a") == true)
		Resources->Current_Camera()->Translate(-10.0f,0,0);
	if(Keyboard::IsKeyDown("d") == true)
		Resources->Current_Camera()->Translate(10.0f,0,0);
	if(Keyboard::IsKeyDown("q") == true)
		Resources->Current_Camera()->Rotate(0,0,0.5f);
	if(Keyboard::IsKeyDown("e") == true)
		Resources->Current_Camera()->Rotate(0,0,-0.5f);

	if(Keyboard::IsKeyDown("1") == true)
		Resources->Objects.at(3)->Translate(1,0,0);
	if(Keyboard::IsKeyDown("2") == true)
		Resources->Objects.at(3)->Translate(-1,0,0);
	if(Keyboard::IsKeyDown("3") == true)
		Resources->Objects.at(3)->Translate(0,1,0);
	if(Keyboard::IsKeyDown("4") == true)
		Resources->Objects.at(3)->Translate(0,-1,0);
	if(Keyboard::IsKeyDown("5") == true)
		Resources->Objects.at(3)->Translate(0,0,1);
	if(Keyboard::IsKeyDown("6") == true)
		Resources->Objects.at(3)->Translate(0,0,-1);

	if(Keyboard::IsKeyDown("f1") == true)
		Resources->Objects.at(4)->Scale(5,5,5);
	if(Keyboard::IsKeyDown("f2") == true)
		Resources->Objects.at(4)->Scale(-5,-5,-5);

	Resources->Current_Camera()->Rotate(Mouse_Difference.y*0.005f,Mouse_Difference.x*0.005f,0);
}
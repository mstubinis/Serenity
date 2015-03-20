#include "Game.h"
#include "Engine.h"
#include "Engine_Events.h"
using namespace Engine;
using namespace Engine::Events;

Game::Game(){}
Game::~Game(){}
void Game::Init_Logic(){

	new Skybox("Wasteland");
	new ObjectDynamic(Resources->Default_Mesh(),Resources->Default_Material(),glm::vec3(0,0,-6));
	new ObjectDynamic(Resources->Get_Mesh("Ground"),Resources->Get_Material("Stone"),glm::vec3(0,-2,0));

	PointLight* pL = new PointLight();

	pL->Position = glm::vec3(0,0,0);
	pL->Color = glm::vec3(1,1,1);
	pL->AmbientIntensity = 0.2f;
	pL->DiffuseIntensity = 0.3f;
	pL->Attenuation.Constant = 0.3f;
	pL->Attenuation.Exp = 0.3f;
	pL->Attenuation.Linear = 0.3f;
	Resources->Lights_Points.push_back(pL);

	PointLight* pL2 = new PointLight();

	pL2->Position = glm::vec3(0,0,-4);
	pL2->Color = glm::vec3(1,1,1);
	pL2->AmbientIntensity = 0.2f;
	pL2->DiffuseIntensity = 0.3f;
	pL2->Attenuation.Constant = 0.3f;
	pL2->Attenuation.Exp = 0.3f;
	pL2->Attenuation.Linear = 0.3f;
	Resources->Lights_Points.push_back(pL2);

	DirectionalLight* dL = new DirectionalLight();

	dL->Color = glm::vec3(1,1,1);
	dL->AmbientIntensity = 0.1f;
	dL->DiffuseIntensity = 0.4f;
	dL->Direction = glm::vec3(-0.5,0.8,0.3);
	Resources->Lights_Directional.push_back(dL);
}
void Game::Init_Resources()
{
}
void Game::Update(float dt){
	if(Keyboard::IsKeyDown("esc"))
		exit(EXIT_SUCCESS);

	if(Keyboard::IsKeyDown("w") == true)
		Resources->Current_Camera()->Translate(0,0,12);
	if(Keyboard::IsKeyDown("s") == true)
		Resources->Current_Camera()->Translate(0,0,-12);
	if(Keyboard::IsKeyDown("a") == true)
		Resources->Current_Camera()->Translate(-12,0,0);
	if(Keyboard::IsKeyDown("d") == true)
		Resources->Current_Camera()->Translate(12,0,0);
	if(Keyboard::IsKeyDown("q") == true)
		Resources->Current_Camera()->Rotate(0,0,0.5f);
	if(Keyboard::IsKeyDown("e") == true)
		Resources->Current_Camera()->Rotate(0,0,-0.5f);

	if(Keyboard::IsKeyDown("1") == true)
		Resources->Lights_Points.at(0)->Position.x += 0.1f;
	if(Keyboard::IsKeyDown("2") == true)
		Resources->Lights_Points.at(0)->Position.x -= 0.1f;
	if(Keyboard::IsKeyDown("3") == true)
		Resources->Lights_Points.at(0)->Position.y += 0.1f;
	if(Keyboard::IsKeyDown("4") == true)
		Resources->Lights_Points.at(0)->Position.y -= 0.1f;
	if(Keyboard::IsKeyDown("5") == true)
		Resources->Lights_Points.at(0)->Position.z += 0.1f;
	if(Keyboard::IsKeyDown("6") == true)
		Resources->Lights_Points.at(0)->Position.z -= 0.1f;

	Resources->Current_Camera()->Rotate(Mouse_Difference.y*0.005f,Mouse_Difference.x*0.005f,0);
}
#include "Engine_Resources.h"
#include "Light.h"
#include "Ship.h"
#include "Engine_Events.h"
#include "GameCamera.h"
#include "Scene.h"

using namespace Engine;
using namespace Engine::Events;

Ship::Ship(std::string mesh, std::string mat, std::string name,glm::vec3 pos, glm::vec3 scl, btCollisionShape* collision,Scene* scene): ObjectDynamic(mesh,mat,pos,scl,name,collision,scene){
	m_WarpFactor = 0;
	m_IsWarping = false;
}
Ship::~Ship()
{
}
void Ship::Update(float dt){
	ObjectDynamic::Update(dt);
}
void Ship::TranslateWarp(float amount){
	float amountToAdd = amount * (1.0f / Mass());
	if((amount > 0 && m_WarpFactor + amount < 1) || (amount < 0 && m_WarpFactor > 0)){
		m_WarpFactor += amountToAdd * Resources::dt();
	}
}

PlayerShip::PlayerShip(std::string mesh, std::string mat, std::string name,glm::vec3 pos, glm::vec3 scl, btCollisionShape* collision,Scene* scene): Ship(mesh,mat,name,pos,scl,collision,scene){
	m_Camera = static_cast<GameCamera*>(Resources::getActiveCamera());
	m_Camera->Follow(this);
}
PlayerShip::~PlayerShip()
{
}
void PlayerShip::Update(float dt){
	if(m_IsWarping && m_WarpFactor > 0){
		float speed = (m_WarpFactor * 1.0f/Mass())*2;

		glm::vec3 s = Forward() * glm::pow(speed,15.0f);

		for(auto obj:Resources::getCurrentScene()->Objects()){
			if(obj.second != this && obj.second->Parent() == nullptr){
				obj.second->Translate(s,false);
			}
		}
		for(auto obj:Resources::getCurrentScene()->Lights()){
			if(obj.second->Parent() == nullptr){
				obj.second->Translate(s,false);
			}
		}
	}

	if(Keyboard::IsKeyDown("w"))
		if(!m_IsWarping)
			Apply_Force(0,0,-1);
		else{
			TranslateWarp(0.1f);
		}
	if(Keyboard::IsKeyDown("s"))
		if(!m_IsWarping)
			Apply_Force(0,0,1);
		else{
			TranslateWarp(-0.1f);
		}
	if(Keyboard::IsKeyDown("a"))
		if(!m_IsWarping)
			Apply_Force(-1,0,0);
	if(Keyboard::IsKeyDown("d"))
		if(!m_IsWarping)
			Apply_Force(1,0,0);

	if(Keyboard::IsKeyDown("f"))
		Apply_Force(0,-1,0);
	if(Keyboard::IsKeyDown("r"))
		Apply_Force(0,1,0);

	if(Keyboard::IsKeyDown("q"))
		Apply_Torque(0,0,1);
	if(Keyboard::IsKeyDown("e"))
		Apply_Torque(0,0,-1);

	if(m_Camera->State() != CAMERA_STATE_ORBIT){
		Apply_Torque(-Mouse::GetMouseDifference().y*0.002f,-Mouse::GetMouseDifference().x*0.002f,0);
	}

	if(Keyboard::IsKeyDownOnce("f1"))
		m_Camera->Follow(this);
	else if(Keyboard::IsKeyDownOnce("f2"))
		m_Camera->Orbit(this);
	else if(Keyboard::IsKeyDownOnce("l")){
		m_IsWarping = !m_IsWarping;
		m_WarpFactor = 0;
	}

	Ship::Update(dt);
}
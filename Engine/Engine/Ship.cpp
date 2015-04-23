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
	m_Target = nullptr;
}
Ship::~Ship(){
}
void Ship::update(float dt){
	ObjectDynamic::update(dt);
}
void Ship::translateWarp(float amount){
	float amountToAdd = amount * (1.0f / getMass());
	if((amount > 0 && m_WarpFactor + amount < 1) || (amount < 0 && m_WarpFactor > 0)){
		m_WarpFactor += amountToAdd * Resources::dt();
	}
}
void Ship::setTarget(Object* target){
	m_Target = target;
}
PlayerShip::PlayerShip(std::string mesh, std::string mat, std::string name,glm::vec3 pos, glm::vec3 scl, btCollisionShape* collision,Scene* scene): Ship(mesh,mat,name,pos,scl,collision,scene){
	m_Camera = static_cast<GameCamera*>(Resources::getActiveCamera());
	m_Camera->follow(this);
}
PlayerShip::~PlayerShip(){
}
void PlayerShip::update(float dt){
	if(m_IsWarping && m_WarpFactor > 0){
		float speed = (m_WarpFactor * 1.0f/getMass())*2;

		glm::vec3 s = getForward() * glm::pow(speed,20.0f);

		for(auto obj:Resources::getCurrentScene()->getObjects()){
			if(obj.second != this && obj.second->getParent() == nullptr){
				obj.second->translate(s,false);
			}
		}
		for(auto obj:Resources::getCurrentScene()->getLights()){
			if(obj.second->getParent() == nullptr){
				obj.second->translate(s,false);
			}
		}
	}
	if(Keyboard::isKeyDown("w"))
		if(!m_IsWarping)
			applyForce(0,0,-1);
		else{
			translateWarp(0.1f);
		}
	if(Keyboard::isKeyDown("s"))
		if(!m_IsWarping)
			applyForce(0,0,1);
		else{
			translateWarp(-0.1f);
		}
	if(Keyboard::isKeyDown("a"))
		if(!m_IsWarping)
			applyForce(-1,0,0);
	if(Keyboard::isKeyDown("d"))
		if(!m_IsWarping)
			applyForce(1,0,0);

	if(Keyboard::isKeyDown("f"))
		applyForce(0,-1,0);
	if(Keyboard::isKeyDown("r"))
		applyForce(0,1,0);

	if(Keyboard::isKeyDown("q"))
		applyTorque(0,0,1);
	if(Keyboard::isKeyDown("e"))
		applyTorque(0,0,-1);

	if(m_Camera->getState() != CAMERA_STATE_ORBIT){
		applyTorque(-Mouse::getMouseDifference().y*0.002f,-Mouse::getMouseDifference().x*0.002f,0);
	}

	if(Keyboard::isKeyDownOnce("f1"))
		m_Camera->follow(this);
	else if(Keyboard::isKeyDownOnce("f2"))
		m_Camera->orbit(this);
	else if(Keyboard::isKeyDownOnce("l")){
		m_IsWarping = !m_IsWarping;
		m_WarpFactor = 0;
	}

	Ship::update(dt);
}
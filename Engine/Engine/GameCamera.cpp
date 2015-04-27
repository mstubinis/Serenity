#include "GameCamera.h"
#include "Engine.h"
#include "Engine_Events.h"
#include "ObjectDisplay.h"

using namespace Engine::Events;

GameCamera::GameCamera(float angle, float ratio, float _near, float _far,Scene* scene):Camera(angle,ratio,_near,_far,scene){
	m_State = CAMERA_STATE_FREEFORM;
	m_Target = nullptr;
	m_OrbitRadius = 0;
}
GameCamera::GameCamera(float left, float right, float bottom, float top, float _near, float _far, Scene* scene):Camera(left,right,bottom,top,_near,_far,scene){
	m_State = CAMERA_STATE_FREEFORM;
	m_Target = nullptr;
	m_OrbitRadius = 0;
}
GameCamera::~GameCamera()
{
}
void GameCamera::update(float dt){
	Camera::update(dt);

	switch(m_State){
		case CAMERA_STATE_FOLLOW:
			m_Position = m_Target->getPosition() + (m_Target->getForward()*glm::length(m_Target->getRadius())*1.7f);
			m_Position += m_Target->getUp() * glm::length(m_Target->getRadius())*0.3f;

			m_Model = glm::mat4(1);
			m_Model = glm::translate(m_Model,m_Position);
			m_Model *= glm::mat4_cast(m_Orientation);

			lookAt(getPosition(),m_Target->getPosition()-(m_Target->getForward()*500.0f),m_Target->getUp());
			break;
		case CAMERA_STATE_ORBIT:
			m_OrbitRadius += Engine::Events::Mouse::getMouseWheelDelta() * dt;
			if( m_OrbitRadius < 0) m_OrbitRadius = 0;
			else if(m_OrbitRadius > 60) m_OrbitRadius = 60;

			rotate(-Mouse::getMouseDifference().y*0.005f,-Mouse::getMouseDifference().x*0.005f,0);

			m_Position = (glm::vec3(0,0,1)*glm::length(m_Target->getRadius())*1.5f) + (glm::vec3(0,0,1)*glm::length(m_Target->getRadius() * (1+m_OrbitRadius)));

			m_Model = glm::mat4(1);
			m_Model = glm::translate(m_Model,m_Target->getPosition());
			m_Model *= glm::mat4_cast(m_Orientation);
			m_Model = glm::translate(m_Model,m_Position);

			lookAt(getPosition(),m_Target->getPosition(),getUp());
			break;
		case CAMERA_STATE_FREEFORM:
			break;
	}
}
void GameCamera::render(){}
void GameCamera::follow(ObjectDisplay* target){
	m_Target = target;
	m_State = CAMERA_STATE_FOLLOW;
}
void GameCamera::orbit(ObjectDisplay* target){
	m_Target = target;
	m_State = CAMERA_STATE_ORBIT;
}
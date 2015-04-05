#include "GameCamera.h"
#include "Engine.h"
#include "Engine_Events.h"

GameCamera::GameCamera(float angle, float ratio, float _near, float _far):Camera(angle,ratio,_near,_far){
	m_State = CAMERA_STATE_FREEFORM;
	m_Target = nullptr;
	m_OrbitRadius = 0;
}
GameCamera::GameCamera(float left, float right, float bottom, float top, float _near, float _far):Camera(left,right,bottom,top,_near,_far){
	m_State = CAMERA_STATE_FREEFORM;
	m_Target = nullptr;
	m_OrbitRadius = 0;
}
GameCamera::~GameCamera()
{
}
void GameCamera::Update(float dt){
	Camera::Update(dt);

	switch(m_State){
		case CAMERA_STATE_FOLLOW:
			m_Position = m_Target->Position() + (m_Target->Forward()*glm::length(m_Target->Radius())*3.0f);
			m_Position += m_Target->Up() * glm::length(m_Target->Radius())*0.6f;
			LookAt(m_Target,true);
			break;
		case CAMERA_STATE_ORBIT:
			m_OrbitRadius += Engine::Events::Mouse::GetMouseWheelDelta() * dt;

			Rotate(Mouse_Difference.y*0.005f,Mouse_Difference.x*0.005f,0);

			m_Position = (m_Target->Forward()*glm::length(m_Target->Radius())*1.5f) + (m_Target->Forward()*glm::length(m_Target->Radius() * (1+m_OrbitRadius)));

			m_Model = glm::mat4(1);
			m_Model = glm::translate(m_Model,m_Target->Position());
			m_Model *= glm::mat4_cast(m_Orientation);
			m_Model = glm::translate(m_Model,m_Position);

			LookAt(Position(),m_Target->Position(),Up());
			break;
		case CAMERA_STATE_FREEFORM:
			break;
	}
}
void GameCamera::Render()
{
}
void GameCamera::Follow(Object* target)
{
	m_Target = target;
	m_State = CAMERA_STATE_FOLLOW;
}
void GameCamera::Orbit(Object* target)
{
	m_Target = target;
	m_State = CAMERA_STATE_ORBIT;
}
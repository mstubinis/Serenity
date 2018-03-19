#include "GameCamera.h"
#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ObjectDisplay.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;



GameCameraComponent::GameCameraComponent(float angle,float aspectRatio,float nearPlane,float farPlane):ComponentCamera(angle,aspectRatio,nearPlane,farPlane){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
}
GameCameraComponent::GameCameraComponent(float left,float right,float bottom,float top,float nearPlane,float farPlane):ComponentCamera(left,right,bottom,top,nearPlane,farPlane){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
}
GameCameraComponent::~GameCameraComponent(){
}
void GameCameraComponent::update(const float& dt){
    switch(m_State){
        case CAMERA_STATE_FOLLOW:{
            m_OrbitRadius += (Engine::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0)     m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            float targetRadius = m_Target->getRadius();


			glm::vec3 pos = m_Target->getPosition() + ((m_Target->getForward() * glm::length(targetRadius) * 1.7f)+ m_Target->getUp() * glm::length(targetRadius) * 0.3f) * (1.0f + m_OrbitRadius);
			pos -= glm::vec3(-0.00001f,-0.00001f,0.00001f);//for some reason this is needed to remove lighting bugs...

			m_Body->setPosition(pos);

            lookAt(
				pos,
				m_Target->getPosition() - m_Target->getForward() * 50.0f,
				m_Target->getUp()
			);
            break;
        }
        case CAMERA_STATE_FOLLOWTARGET:{
            m_OrbitRadius += (Engine::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0)     m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,m_Player->getPosition() -
                ((glm::normalize(m_Target->getPosition() - m_Player->getPosition())*(m_Player->getRadius()*2.7f)* (1.0f + m_OrbitRadius))
                - m_Player->getUp() * glm::length(m_Player->getRadius())*0.3f));

			glm::vec3 pos(model[3][0],model[3][1],model[3][2]);

			m_Body->setPosition(pos);

            lookAt(
				pos,
				m_Target->getPosition(),
				m_Player->getUp()
			);
            break;
        }
        case CAMERA_STATE_ORBIT:{
            m_OrbitRadius += Engine::getMouseWheelDelta() * 0.01f;
            if( m_OrbitRadius < 0)      m_OrbitRadius = 0;
            else if(m_OrbitRadius > 60) m_OrbitRadius = 60;

            m_Body->rotate(-Engine::getMouseDifference().y * 0.02f * dt, -Engine::getMouseDifference().x * 0.02f * dt,0);

            glm::vec3 pos = (glm::vec3(0,0,1) * glm::length(m_Target->getRadius()) * 0.37f) + (glm::vec3(0,0,1) * glm::length(m_Target->getRadius() * (1.0f + m_OrbitRadius)));

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,m_Target->getPosition());
            model *= glm::mat4_cast(m_Body->rotation());
            model = glm::translate(model,pos);

			pos = glm::vec3(model[3][0],model[3][1],model[3][2]);

			m_Body->setPosition(pos);

            lookAt(
				pos,
				m_Target->getPosition(),
				m_Body->up()
			);
            break;
        }
        case CAMERA_STATE_FREEFORM:{
            lookAt(m_Body->position(),m_Body->position() + m_Body->forward(), m_Body->up());
            break;
        }
    }
}



GameCamera::GameCamera(float a, float r, float n, float f,Scene* scene):Camera(a,r,n,f,scene){
	removeComponent(m_Camera);
	m_Camera = new GameCameraComponent(a,r,n,f);
	addComponent(m_Camera);
	
	((GameCameraComponent*)m_Camera)->m_Body = m_BasicBody;
}
GameCamera::GameCamera(float l, float r, float b, float t, float n, float f, Scene* scene):Camera(l,r,b,t,n,f,scene){
	removeComponent(m_Camera);
	m_Camera = new GameCameraComponent(l,r,b,t,n,f);
	addComponent(m_Camera);

	((GameCameraComponent*)m_Camera)->m_Body = m_BasicBody;
}
GameCamera::~GameCamera()
{
}
void GameCamera::update(const float& dt){

}
Object* GameCamera::getObjectInCenterRay(Object* exclusion){
    Object* ret = nullptr;
    vector<Object*> objs;
    for(auto object:Engine::Resources::getCurrentScene()->objects()){
        if(object.second->rayIntersectSphere(this)){
            if(object.second != exclusion)
                objs.push_back(object.second);
        }
    }
    if(objs.size() == 0) return nullptr;
    if(objs.size() == 1) return objs.at(0);

    float distance = -1;
    for(auto object:objs){
        float d = glm::distance(object->getPosition(), getPosition());
        if(distance == -1 || d < distance){
            distance = d;
            ret = object;
        }
    }
    return ret;
}
void GameCamera::render(){}
void GameCamera::follow(Object* target){
	GameCameraComponent& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = target;
    cam.m_State = CAMERA_STATE_FOLLOW;
}
void GameCamera::followTarget(Object* target,Object* player){
	GameCameraComponent& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = player;
    cam.m_State = CAMERA_STATE_FOLLOWTARGET;
}
void GameCamera::orbit(Object* target){
	GameCameraComponent& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = target;
    cam.m_State = CAMERA_STATE_ORBIT;
}
void GameCamera::setTarget(Object* target) { ((GameCameraComponent*)m_Camera)->m_Target = target; }
const Object* GameCamera::getTarget() const { return ((GameCameraComponent*)m_Camera)->m_Target; }
const CAMERA_STATE GameCamera::getState() const { return ((GameCameraComponent*)m_Camera)->m_State; }
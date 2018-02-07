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

GameCamera::GameCamera(string name, float a, float r, float n, float f,Scene* scene):Camera(name,a,r,n,f,scene){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
}
GameCamera::GameCamera(string name, float l, float r, float b, float t, float n, float f, Scene* scene):Camera(name,l,r,b,t,n,f,scene){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
}
GameCamera::~GameCamera()
{
}
void GameCamera::update(float dt){
    switch(m_State){
        case CAMERA_STATE_FOLLOW:{
            m_OrbitRadius += (Engine::Events::Mouse::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0) m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            float targetRadius = m_Target->getRadius();

            m_Model = Renderer::Detail::RenderManagement::m_IdentityMat4;
            m_Model = glm::translate(m_Model,m_Target->getPosition() + ((m_Target->getForward()*glm::length(targetRadius)*1.7f)+ m_Target->getUp()*glm::length(targetRadius)*0.3f)*(1.0f + m_OrbitRadius));
            m_Model *= glm::mat4_cast(m_Orientation);
            m_Model = glm::translate(m_Model,glm::vec3(-0.00001f,-0.00001f,0.00001f)); //for some reason this is needed to remove lighting bugs...

            lookAt(getPosition(),m_Target->getPosition()-(m_Target->getForward()*50.0f),m_Target->getUp());
            break;
        }
        case CAMERA_STATE_FOLLOWTARGET:{
            m_OrbitRadius += (Engine::Events::Mouse::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0) m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            m_Model = Renderer::Detail::RenderManagement::m_IdentityMat4;
            m_Model = glm::translate(m_Model,m_Player->getPosition() -
                ((glm::normalize(m_Target->getPosition() - m_Player->getPosition())*(m_Player->getRadius()*2.7f)* (1.0f + m_OrbitRadius))
                - m_Player->getUp() * glm::length(m_Player->getRadius())*0.3f));

            lookAt(getPosition(),m_Target->getPosition(),m_Player->getUp());
            break;
        }
        case CAMERA_STATE_ORBIT:{
            m_OrbitRadius += Engine::Events::Mouse::getMouseWheelDelta() * 0.01f;
            if( m_OrbitRadius < 0) m_OrbitRadius = 0;
            else if(m_OrbitRadius > 60) m_OrbitRadius = 60;

            rotate(-Events::Mouse::getMouseDifference().y*0.02f,-Events::Mouse::getMouseDifference().x*0.02f,0);

            glm::vec3 pos = (glm::vec3(0,0,1)*glm::length(m_Target->getRadius())*0.37f) + (glm::vec3(0,0,1)*glm::length(m_Target->getRadius() * (1.0f + m_OrbitRadius)));

            m_Model = Renderer::Detail::RenderManagement::m_IdentityMat4;
            m_Model = glm::translate(m_Model,m_Target->getPosition());
            m_Model *= glm::mat4_cast(m_Orientation);
            m_Model = glm::translate(m_Model,pos);

            lookAt(getPosition(),m_Target->getPosition(),getUp());
            break;
        }
        case CAMERA_STATE_FREEFORM:{
            lookAt(getPosition(),getPosition() + getForward(), getUp());
            break;
        }
    }
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
        float d = glm::distance(object->getPosition(),this->getPosition());
        if(distance == -1 || d < distance){
            distance = d;
            ret = object;
        }
    }
    return ret;
}
void GameCamera::render(){}
void GameCamera::follow(Object* target){
    m_Target = target;
    m_Player = target;
    m_State = CAMERA_STATE_FOLLOW;
}
void GameCamera::followTarget(Object* target,Object* player){
    m_Target = target;
    m_Player = player;
    m_State = CAMERA_STATE_FOLLOWTARGET;
}
void GameCamera::orbit(Object* target){
    m_Target = target;
    m_Player = target;
    m_State = CAMERA_STATE_ORBIT;
}

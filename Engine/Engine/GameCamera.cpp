#include "GameCamera.h"
#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "ObjectDisplay.h"
#include "Scene.h"

using namespace Engine::Events;

GameCamera::GameCamera(std::string name, float a, float r, float n, float f,Scene* scene):Camera(name,a,r,n,f,scene){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
}
GameCamera::GameCamera(std::string name, float l, float r, float b, float t, float n, float f, Scene* scene):Camera(name,l,r,b,t,n,f,scene){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
}
GameCamera::~GameCamera()
{
}
void GameCamera::update(float dt){
    Camera::update(dt);
    switch(m_State){
        case CAMERA_STATE_FOLLOW:{
            m_OrbitRadius += (Engine::Events::Mouse::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0) m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            m_Model = glm::m4(1);
            m_Model = glm::translate(m_Model,m_Target->getPosition()+((glm::v3(m_Target->getForward())*static_cast<glm::nType>(glm::length(m_Target->getRadius()))*static_cast<glm::nType>(2.0))+ glm::v3(m_Target->getUp()) * static_cast<glm::nType>(glm::length(m_Target->getRadius()))*static_cast<glm::nType>(0.3))* (static_cast<glm::nType>(1.0)+m_OrbitRadius));

            lookAt(getPosition(),m_Target->getPosition()-(glm::v3(m_Target->getForward())*static_cast<glm::nType>(500.0)),glm::v3(m_Target->getUp()));
            break;
        }
        case CAMERA_STATE_FOLLOWTARGET:{
            m_OrbitRadius += (Engine::Events::Mouse::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0) m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            m_Model = glm::mat4(1);
            m_Model = glm::translate(m_Model,m_Player->getPosition() -
                                            ((glm::normalize(m_Target->getPosition() - m_Player->getPosition())*(m_Player->getRadius()*static_cast<glm::nType>(2.7))* (static_cast<glm::nType>(1.0)+m_OrbitRadius))
                                            - glm::v3(m_Player->getUp()) * static_cast<glm::nType>(glm::length(m_Player->getRadius()))*static_cast<glm::nType>(0.3)));

            lookAt(getPosition(),m_Target->getPosition(),glm::v3(m_Player->getUp()));
            break;
        }
        case CAMERA_STATE_ORBIT:{
            m_OrbitRadius += Engine::Events::Mouse::getMouseWheelDelta() * 0.01f;
            if( m_OrbitRadius < 0) m_OrbitRadius = 0;
            else if(m_OrbitRadius > 60) m_OrbitRadius = 60;

            rotate(-Mouse::getMouseDifference().y*0.6f,-Mouse::getMouseDifference().x*0.6f,0);

            glm::v3 pos = (glm::v3(0,0,1)*static_cast<glm::nType>(glm::length(m_Target->getRadius()))*static_cast<glm::nType>(0.37)) + (glm::v3(0,0,1)*static_cast<glm::nType>(glm::length(m_Target->getRadius()) * (1.0+m_OrbitRadius)));

            m_Model = glm::m4(1);
            m_Model = glm::translate(m_Model,m_Target->getPosition());
            m_Model *= glm::m4(glm::mat4_cast(m_Orientation));
            m_Model = glm::translate(m_Model,pos);

            lookAt(getPosition(),m_Target->getPosition(),glm::v3(getUp()));
            break;
        }
        case CAMERA_STATE_FREEFORM:{
            break;
        }
    }
}
Object* GameCamera::getObjectInCenterRay(Object* exclusion){
    Object* ret = nullptr;
    std::vector<Object*> objs;
    for(auto object:Engine::Resources::getCurrentScene()->getObjects()){
        if(object.second->rayIntersectSphere(this)){
            if(object.second != exclusion)
                objs.push_back(object.second);
        }
    }
    if(objs.size() == 0)
        return nullptr;
    if(objs.size() == 1)
        return objs.at(0);

    glm::nType distance = -1;
    for(auto object:objs){
        glm::nType d = glm::distance(object->getPosition(),this->getPosition());
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
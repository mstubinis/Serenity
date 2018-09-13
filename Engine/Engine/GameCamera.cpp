#include "GameCamera.h"
#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
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
            auto& targetBody = *(m_Target->getComponent<ComponentBody>());
            auto& targetModel = *(m_Target->getComponent<ComponentModel>());
            float targetRadius = targetModel.radius();

            m_OrbitRadius += (Engine::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0)     m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            glm::vec3 pos = targetBody.position() + ((targetBody.forward() * glm::length(targetRadius) * 1.7f)+ targetBody.up() * glm::length(targetRadius) * 0.3f) * (1.0f + m_OrbitRadius);
            pos -= glm::vec3(-0.00001f,-0.00001f,0.00001f);//for some reason this is needed to remove lighting bugs...

            m_Body->setPosition(pos);

            lookAt(pos,targetBody.position() - targetBody.forward() * 50.0f,targetBody.up());
            break;
        }
        case CAMERA_STATE_FOLLOWTARGET:{
            auto& target = *(m_Target->getComponent<ComponentBody>());
            auto& player = *(m_Player->getComponent<ComponentBody>());
            auto& playerModel = *(m_Player->getComponent<ComponentModel>());

            m_OrbitRadius += (Engine::getMouseWheelDelta() * 0.02f);
            if( m_OrbitRadius < 0)     m_OrbitRadius = 0;
            else if(m_OrbitRadius > 3) m_OrbitRadius = 3;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,player.position() -
                ((glm::normalize(target.position() - player.position())*(playerModel.radius() * 2.7f) * (1.0f + m_OrbitRadius))
                - player.up() * glm::length(playerModel.radius())*0.3f));

            glm::vec3 pos(model[3][0],model[3][1],model[3][2]);

            m_Body->setPosition(pos);

            lookAt(pos,target.position(),player.up());
            break;
        }
        case CAMERA_STATE_ORBIT:{
            auto& targetBody = *(m_Target->getComponent<ComponentBody>());
            auto& targetModel = *(m_Target->getComponent<ComponentModel>());


            m_OrbitRadius += Engine::getMouseWheelDelta() * 0.01f;
            if( m_OrbitRadius < 0)      m_OrbitRadius = 0;
            else if(m_OrbitRadius > 70) m_OrbitRadius = 70;

            m_Body->rotate(-Engine::getMouseDifference().y * 0.02f * dt, -Engine::getMouseDifference().x * 0.02f * dt,0);

            glm::vec3 pos = (glm::vec3(0,0,1) * glm::length(targetModel.radius()) * 0.37f) + (glm::vec3(0,0,1) * glm::length(targetModel.radius() * (1.0f + m_OrbitRadius)));

            glm::mat4 cameraModel = glm::mat4(1.0f);
            cameraModel = glm::translate(cameraModel,targetBody.position());
            cameraModel *= glm::mat4_cast(m_Body->rotation());
            cameraModel = glm::translate(cameraModel,pos);

            glm::vec3 eye(cameraModel[3][0],cameraModel[3][1],cameraModel[3][2]);
            eye -= glm::vec3(-0.00001f,-0.00001f,0.00001f);//for some reason this is needed to remove lighting bugs...
            m_Body->setPosition(eye);

            lookAt(eye,targetBody.position(),m_Body->up());
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
    
    ((GameCameraComponent*)m_Camera)->m_Body = m_Body;
}
GameCamera::GameCamera(float l, float r, float b, float t, float n, float f, Scene* scene):Camera(l,r,b,t,n,f,scene){
    removeComponent(m_Camera);
    m_Camera = new GameCameraComponent(l,r,b,t,n,f);
    addComponent(m_Camera);

    ((GameCameraComponent*)m_Camera)->m_Body = m_Body;
}
GameCamera::~GameCamera()
{
}
void GameCamera::update(const float& dt){

}

Entity* GameCamera::getObjectInCenterRay(Entity* exclusion){
    Entity* ret = 0;
    vector<Entity*> objs;
    Scene* s = Resources::getCurrentScene();
    for(auto id:epriv::InternalScenePublicInterface::GetEntities(s)){
        Entity* e = s->getEntity(id);
        if(rayIntersectSphere(e)){
            if(e != exclusion){
                objs.push_back(e);
            }
        }
    }
    if(objs.size() == 0) return nullptr;
    if(objs.size() == 1) return objs.at(0);

    float distance = -1;
    for(auto object:objs){
        auto& body = *(object->getComponent<ComponentBody>());
        float d = glm::distance(body.position(), getPosition());
        if(distance == -1 || d < distance){
            distance = d;
            ret = object;
        }
    }
    return ret;
}
void GameCamera::render(){}
void GameCamera::follow(Entity* target){
    GameCameraComponent& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = target;
    cam.m_State = CAMERA_STATE_FOLLOW;
}
void GameCamera::followTarget(Entity* target,Entity* player){
    auto& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = player;
    cam.m_State = CAMERA_STATE_FOLLOWTARGET;
}
void GameCamera::orbit(Entity* target){
    auto& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = target;
    cam.m_State = CAMERA_STATE_ORBIT;
}
void GameCamera::setTarget(Entity* target) { ((GameCameraComponent*)m_Camera)->m_Target = target; }
const Entity* GameCamera::getTarget() const { return ((GameCameraComponent*)m_Camera)->m_Target; }
const CAMERA_STATE GameCamera::getState() const { return ((GameCameraComponent*)m_Camera)->m_State; }
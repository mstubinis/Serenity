#include "GameCamera.h"
#include "core/engine/Engine_Events.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Renderer.h"
#include "core/Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;

GameCameraComponent::GameCameraComponent(float angle,float aspectRatio,float nearPlane,float farPlane):OLD_ComponentCamera(angle,aspectRatio,nearPlane,farPlane){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
    m_CameraMouseFactor = glm::vec2(0.0f);
}
GameCameraComponent::GameCameraComponent(float left,float right,float bottom,float top,float nearPlane,float farPlane):OLD_ComponentCamera(left,right,bottom,top,nearPlane,farPlane){
    m_State = CAMERA_STATE_FREEFORM;
    m_Target = nullptr;
    m_Player = nullptr;
    m_CameraMouseFactor = glm::vec2(0.0f);
}
GameCameraComponent::~GameCameraComponent(){
}
void GameCameraComponent::update(const float& dt){
    switch(m_State){
        case CAMERA_STATE_FOLLOW:{
            auto& targetBody = *(m_Target->getComponent<OLD_ComponentBody>());
            auto& targetModel = *(m_Target->getComponent<OLD_ComponentModel>());
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
            auto& target = *(m_Target->getComponent<OLD_ComponentBody>());
            auto& player = *(m_Player->getComponent<OLD_ComponentBody>());
            auto& playerModel = *(m_Player->getComponent<OLD_ComponentModel>());

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
            auto& targetBody = *(m_Target->getComponent<OLD_ComponentBody>());
            auto& targetModel = *(m_Target->getComponent<OLD_ComponentModel>());


            m_OrbitRadius += Engine::getMouseWheelDelta() * 0.01f;
            if( m_OrbitRadius < 0)      m_OrbitRadius = 0;
            else if(m_OrbitRadius > 70) m_OrbitRadius = 70;

            m_CameraMouseFactor += glm::vec2(-Engine::getMouseDifference().y * 0.02f, -Engine::getMouseDifference().x * 0.02f);

            m_Body->rotate(m_CameraMouseFactor.x * dt, m_CameraMouseFactor.y * dt,0);

            m_CameraMouseFactor *= 0.93f;

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

OLD_Entity* GameCamera::getObjectInCenterRay(OLD_Entity* exclusion){
    OLD_Entity* ret = 0;
    vector<OLD_Entity*> objs;
    Scene& s = *Resources::getCurrentScene();
    for(auto id:epriv::InternalScenePublicInterface::OLD_GetEntities(s)){
        OLD_Entity* e = s.OLD_getEntity(id);
        if(rayIntersectSphere(e)){
            if(e != exclusion){
                objs.push_back(e);
            }
        }
    }
    if(objs.size() == 0) return nullptr;
    if(objs.size() == 1) return objs[0];

    float distance = -1;
    for(auto object:objs){
        auto& body = *(object->getComponent<OLD_ComponentBody>());
        float d = glm::distance(body.position(), getPosition());
        if(distance == -1 || d < distance){
            distance = d;
            ret = object;
        }
    }
    return ret;
}
void GameCamera::render(){}
void GameCamera::follow(OLD_Entity* target){
    GameCameraComponent& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = target;
    cam.m_State = CAMERA_STATE_FOLLOW;
}
void GameCamera::followTarget(OLD_Entity* target, OLD_Entity* player){
    auto& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = player;
    cam.m_State = CAMERA_STATE_FOLLOWTARGET;
}
void GameCamera::orbit(OLD_Entity* target){
    auto& cam = *((GameCameraComponent*)m_Camera);
    cam.m_Target = target;
    cam.m_Player = target;
    cam.m_State = CAMERA_STATE_ORBIT;
    cam.m_CameraMouseFactor = glm::vec2(0.0f);
}
void GameCamera::setTarget(OLD_Entity* target) { ((GameCameraComponent*)m_Camera)->m_Target = target; }
const OLD_Entity* GameCamera::getTarget() const { return ((GameCameraComponent*)m_Camera)->m_Target; }
const CAMERA_STATE GameCamera::getState() const { return ((GameCameraComponent*)m_Camera)->m_State; }
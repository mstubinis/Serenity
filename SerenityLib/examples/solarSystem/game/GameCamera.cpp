#include "GameCamera.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/scene/Scene.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;


struct GameCameraLogicFunctor final { void operator()(ComponentLogic2& _component, const double& dt) const {
    GameCamera& camera = *((GameCamera*)_component.getUserPointer());
    auto& entity = camera.m_Entity;
    EntityDataRequest dataRequest(entity);
    auto& thisCamera = *entity.getComponent<ComponentCamera>(dataRequest);
    auto& thisBody   = *entity.getComponent<ComponentBody>(dataRequest);

    switch (camera.m_State) {
        case CameraState::Follow: {
            auto& targetEntity = camera.m_Target;
            EntityDataRequest dataRequest1(targetEntity);

            auto& targetBody   = *targetEntity.getComponent<ComponentBody>(dataRequest1);
            auto& targetModel  = *targetEntity.getComponent<ComponentModel>(dataRequest1);
            float targetRadius = targetModel.radius();

            camera.m_OrbitRadius += (Engine::getMouseWheelDelta() * 0.02f);
			camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 3.0f);

            glm::vec3 pos = targetBody.position() + ((targetBody.forward() * glm::length(targetRadius) * 1.7f) + targetBody.up() * glm::length(targetRadius) * 0.3f) * (1.0f + camera.m_OrbitRadius);

            thisBody.setPosition(pos);

            thisCamera.lookAt(pos, targetBody.position() - targetBody.forward() * 50.0f, targetBody.up());
            break;
        }
        case CameraState::FollowTarget: {
            auto& targetEntity = camera.m_Target;
            auto& playerEntity = camera.m_Player;
            EntityDataRequest dataRequest1(targetEntity);
            EntityDataRequest dataRequest2(playerEntity);

            auto& target = *targetEntity.getComponent<ComponentBody>(dataRequest1);
            auto& player = *playerEntity.getComponent<ComponentBody>(dataRequest2);
            auto& playerModel = *playerEntity.getComponent<ComponentModel>(dataRequest2);

            camera.m_OrbitRadius += (Engine::getMouseWheelDelta() * 0.02f);
			camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 3.0f);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, player.position() -
                ((glm::normalize(target.position() - player.position())*(playerModel.radius() * 2.7f) * (1.0f + camera.m_OrbitRadius))
                    - player.up() * glm::length(playerModel.radius())*0.3f));

            glm::vec3 pos(model[3][0], model[3][1], model[3][2]);

            thisBody.setPosition(pos);

            thisCamera.lookAt(pos, target.position(), player.up());
            break;
        }
        case CameraState::Orbit: {
            auto& targetEntity = camera.m_Target;
            EntityDataRequest dataRequest1(targetEntity);

            auto& targetBody = *targetEntity.getComponent<ComponentBody>(dataRequest1);
            auto& targetModel = *targetEntity.getComponent<ComponentModel>(dataRequest1);

			camera.m_OrbitRadius += Engine::getMouseWheelDelta() * dt * 0.92f;
			camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 70.0f);

			const auto& diff = Engine::getMouseDifference();
			camera.m_CameraMouseFactor += glm::dvec2(diff.y * (dt * 0.1), -diff.x * (dt * 0.1));

			thisBody.rotate(camera.m_CameraMouseFactor.x, camera.m_CameraMouseFactor.y, 0);
			const double& step = (1.0 - dt);
			camera.m_CameraMouseFactor *= (step * 0.997);

            const glm::vec3& pos = (glm::vec3(0, 0, 1) * glm::length(targetModel.radius()) * 0.37f) + (glm::vec3(0, 0, 1) * glm::length(targetModel.radius() * (1.0f + camera.m_OrbitRadius)));

            glm::mat4 cameraModel = glm::mat4(1.0f);
            cameraModel = glm::translate(cameraModel, targetBody.position());
            cameraModel *= glm::mat4_cast(thisBody.rotation());
            cameraModel = glm::translate(cameraModel, pos);

            const glm::vec3 eye(cameraModel[3][0], cameraModel[3][1], cameraModel[3][2]);
            thisBody.setPosition(eye);

            thisCamera.lookAt(eye, targetBody.position(), thisBody.up());
            break;
        }
        case CameraState::Freeform: {
            thisCamera.lookAt(thisBody.position(), thisBody.position() + thisBody.forward(), thisBody.up());
            break;
        }
    }
} };

#pragma region GameCamera


GameCamera::GameCamera(float n, float f, Scene* scene):GameCamera(60,Resources::getWindowSize().x / static_cast<float>(Resources::getWindowSize().y),n,f,scene) {

}
GameCamera::GameCamera(float a, float r, float n, float f,Scene* scene):Camera(a,r,n,f,scene){
    m_State = CameraState::Freeform;
    m_Target = Entity::_null;
    m_Player = Entity::_null;
    m_OrbitRadius = 0;
    m_CameraMouseFactor = glm::dvec2(0.0);
    auto& m_Logic = *m_Entity.getComponent<ComponentLogic2>();
    m_Logic.setUserPointer(this);
    m_Logic.setFunctor(GameCameraLogicFunctor());
}
GameCamera::GameCamera(float l, float r, float b, float t, float n, float f, Scene* scene):Camera(l,r,b,t,n,f,scene){
    m_State = CameraState::Freeform;
    m_Target = Entity::_null;
    m_Player = Entity::_null;
    m_OrbitRadius = 0;
    m_CameraMouseFactor = glm::dvec2(0.0);
    auto& m_Logic = *m_Entity.getComponent<ComponentLogic2>();
    m_Logic.setUserPointer(this);
    m_Logic.setFunctor(GameCameraLogicFunctor());
}
GameCamera::~GameCamera(){
}
Entity GameCamera::getObjectInCenterRay(Entity& exclusion){
    Entity ret = Entity::_null;
    vector<Entity> objs;
    Scene& s = *Resources::getCurrentScene();
    for(auto& data:epriv::InternalScenePublicInterface::GetEntities(s)){
        Entity e = s.getEntity(data);
        if(rayIntersectSphere(e)){
            if(e != exclusion){
                objs.push_back(e);
            }
        }
    }
    if(objs.size() == 0) return ret;
    if(objs.size() == 1) return objs[0];

    float distance = -1;
    for(auto& object:objs){
        auto* body = object.getComponent<ComponentBody>();
        if (body) {
            float d = glm::distance(body->position(), getPosition());
            if (distance == -1 || d < distance) {
                distance = d;
                ret = object;
            }
        }
    }
    return ret;
}
void GameCamera::follow(Entity& target){
    if (target.null()) return;
    m_Target = target;
    m_Player = target;
    m_State = CameraState::Follow;
}
void GameCamera::followTarget(Entity& target, Entity& player){
    if (target.null()) return;
    m_Target = target;
    m_Player = player;
    m_State = CameraState::FollowTarget;
}
void GameCamera::orbit(Entity& target){
    if (target.null()) return;
    m_Target = target;
    m_Player = target;
    m_State = CameraState::Orbit;
    m_CameraMouseFactor = glm::dvec2(0.0);
}
void GameCamera::setTarget(Entity& target) { 
    m_Target = target; 
}
const Entity& GameCamera::getTarget() const { 
    return m_Target; 
}
const CameraState::State GameCamera::getState() const { 
    return m_State; 
}

#pragma endregion

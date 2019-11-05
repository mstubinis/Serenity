#include "GameCamera.h"
#include "Ship.h"
#include "map/Map.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/scene/Scene.h>

#include "ships/shipSystems/ShipSystemSensors.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace std;


struct GameCameraLogicFunctor final { void operator()(ComponentLogic2& _component, const double& dt) const {
    GameCamera& camera = *((GameCamera*)_component.getUserPointer());
    auto& entity = camera.m_Entity;
    EntityDataRequest dataRequest(entity);
    auto& thisCamera = *entity.getComponent<ComponentCamera>(dataRequest);
    auto& thisBody   = *entity.getComponent<ComponentBody>(dataRequest);

    switch (camera.m_State) {
        case CameraState::Cockpit: {
            auto& targetEntity = camera.m_Player;
            EntityDataRequest dataRequest1(targetEntity->entity());

            auto& targetBody   = *targetEntity->getComponent<ComponentBody>(dataRequest1);
            auto& targetModel  = *targetEntity->getComponent<ComponentModel>(dataRequest1);
            float targetRadius = targetModel.radius();
            const auto targetFwd = glm::vec3(targetBody.forward());
            const auto targetPos = glm::vec3(targetBody.position());
            const auto targetUp = glm::vec3(targetBody.up());

            camera.m_OrbitRadius += (static_cast<float>(Engine::getMouseWheelDelta()) * 0.02f);
			camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 3.0f);

            auto calc2 = ((targetFwd * glm::length(targetRadius) * 1.7f) + targetUp * glm::length(targetRadius) * 0.3f);
            
            auto* ship = dynamic_cast<Ship*>(targetEntity);
            if (ship)
                calc2 += (glm::quat(targetBody.rotation()) * ship->m_CameraOffsetDefault);
            calc2 *= (1.0f + camera.m_OrbitRadius);

            const auto pos = targetPos + calc2;

            thisBody.setPosition(pos);
            thisCamera.lookAt(pos, targetPos - (glm::vec3(50000.0f) * targetFwd), targetUp);
            break;
        }case CameraState::FollowTarget: {
            auto& targetEntity = camera.m_Target;
            auto& playerEntity = camera.m_Player;
            EntityDataRequest dataRequest1(targetEntity->entity());
            EntityDataRequest dataRequest2(playerEntity->entity());

            auto& target = *targetEntity->getComponent<ComponentBody>(dataRequest1);
            auto& player = *playerEntity->getComponent<ComponentBody>(dataRequest2);
            auto& playerModel = *playerEntity->getComponent<ComponentModel>(dataRequest2);

            camera.m_OrbitRadius += (static_cast<float>(Engine::getMouseWheelDelta()) * 0.02f);
			camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 3.0f);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(player.position()) -
                ((glm::normalize(glm::vec3(target.position()) - glm::vec3(player.position()))*(playerModel.radius() * 2.7f) * (1.0f + camera.m_OrbitRadius))
                    - glm::vec3(player.up()) * glm::length(playerModel.radius()) * 0.55f));

            glm::vec3 pos(model[3][0], model[3][1], model[3][2]);

            thisBody.setPosition(pos);

            thisCamera.lookAt(pos, target.position(), player.up());
            break;
        }case CameraState::Orbit: {
            auto& targetEntity = camera.m_Target;
            EntityDataRequest dataRequest1(targetEntity->entity());

            auto& targetBody = *targetEntity->getComponent<ComponentBody>(dataRequest1);
            auto& targetModel = *targetEntity->getComponent<ComponentModel>(dataRequest1);

			camera.m_OrbitRadius += static_cast<float>(Engine::getMouseWheelDelta()) * static_cast<float>(dt) * 0.92f;
			camera.m_OrbitRadius = glm::clamp(camera.m_OrbitRadius, 0.0f, 70.0f);

			const auto& diff = Engine::getMouseDifference();
            auto x_amount = (diff.y * 0.8) * (dt * 0.1);
            auto y_amount = (-diff.x * 0.8) * (dt * 0.1);
            camera.m_CameraMouseFactor.x += x_amount;
            camera.m_CameraMouseFactor.x = glm::clamp(camera.m_CameraMouseFactor.x, -0.06, 0.06);
            camera.m_CameraMouseFactor.y += y_amount;
            camera.m_CameraMouseFactor.y = glm::clamp(camera.m_CameraMouseFactor.y, -0.06, 0.06);

			thisBody.rotate(camera.m_CameraMouseFactor.x, camera.m_CameraMouseFactor.y, 0);
			const double& step = (1.0 - dt);
			camera.m_CameraMouseFactor *= (step * 0.997);

            const glm::vec3& pos = (glm::vec3(0, 0, 1) * glm::length(targetModel.radius()) * 0.37f) + (glm::vec3(0, 0, 1) * glm::length(targetModel.radius() * (1.0f + camera.m_OrbitRadius)));

            glm::mat4 cameraModel = glm::mat4(1.0f);
            cameraModel = glm::translate(cameraModel, glm::vec3(targetBody.position()));
            cameraModel *= glm::mat4_cast(glm::quat(thisBody.rotation()));
            cameraModel = glm::translate(cameraModel, pos);

            const glm::vec3 eye(cameraModel[3][0], cameraModel[3][1], cameraModel[3][2]);
            thisBody.setPosition(eye);

            thisCamera.lookAt(eye, targetBody.position(), thisBody.up());
            break;
        }case CameraState::Freeform: {
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
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
    m_CameraMouseFactor = glm::dvec2(0.0);
    auto& m_Logic = *getComponent<ComponentLogic2>();
    m_Logic.setUserPointer(this);
    m_Logic.setFunctor(GameCameraLogicFunctor());
}
GameCamera::GameCamera(float l, float r, float b, float t, float n, float f, Scene* scene):Camera(l,r,b,t,n,f,scene){
    m_State = CameraState::Freeform;
    m_Target = nullptr;
    m_Player = nullptr;
    m_OrbitRadius = 0;
    m_CameraMouseFactor = glm::dvec2(0.0);
    auto& m_Logic = *getComponent<ComponentLogic2>();
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

    decimal distance = -1;
    for(auto& object:objs){
        auto* body = object.getComponent<ComponentBody>();
        if (body) {
            decimal d = glm::distance(body->position(), getPosition());
            if (distance == -1 || d < distance) {
                distance = d;
                ret = object;
            }
        }
    }
    return ret;
}

const bool GameCamera::validateDistanceForOrbit(Map& map) {
    auto* ship = dynamic_cast<Ship*>(m_Player);
    if (ship) {
        auto* sensors = static_cast<ShipSystemSensors*>(ship->getShipSystem(ShipSystemType::Sensors));
        auto* target = sensors->getTarget();
        if (target && target != m_Player) {
            const auto dist2 = glm::distance2(ship->getPosition(), target->getComponent<ComponentBody>()->position());
            if (dist2 < static_cast<decimal>(10000000000.0)) { //to prevent FP issues when viewing things billions of km away
                map.centerSceneToObject(m_Target->entity());
                m_Target = target;
                return true;
            }
        }
    }
    m_Target = m_Player;
    return false;
}

void GameCamera::setState(const CameraState::State& new_state){
    const auto old_state = m_State;
    m_State = new_state;
    Map& map = static_cast<Map&>(m_Entity.scene());
    if (new_state == CameraState::Orbit) {
        m_CameraMouseFactor = glm::dvec2(0.0);

        if (old_state == CameraState::Cockpit || old_state == CameraState::FollowTarget) {
            validateDistanceForOrbit(map);
            map.centerSceneToObject(m_Player->entity());
        }else{
            if (!m_Target || (m_Target && m_Target == m_Player)) {
                validateDistanceForOrbit(map);
            }else if(m_Player && m_Target && (m_Target != m_Player)){
                map.centerSceneToObject(m_Player->entity());
                m_Target = m_Player;
            }
        }

    }else if (new_state == CameraState::Cockpit) {
        map.centerSceneToObject(m_Player->entity());
    }else if (new_state == CameraState::FollowTarget) {
        auto* ship = dynamic_cast<Ship*>(m_Player);
        if (ship) {
            auto* sensors = static_cast<ShipSystemSensors*>(ship->getShipSystem(ShipSystemType::Sensors));
            auto* target = sensors->getTarget();
            if (target) {
                m_Target = target;
            }else{
                m_Target = m_Player;
                if (old_state == CameraState::FollowTarget) {
                    m_State = CameraState::Cockpit;
                }else{
                    m_State = old_state;
                }
                return;
            }
        }
        map.centerSceneToObject(m_Player->entity());
    }
}
void GameCamera::setTarget(EntityWrapper* target) {
    m_Target = target; 
    setState(m_State);
}
void GameCamera::setTarget(const string& targetName) {
    if (targetName.empty()) {
        setTarget(nullptr);
    }
    //if (m_Player) {
        Map& map = static_cast<Map&>(m_Player->entity().scene());
        for (auto& entity : map.m_Objects) {
            auto* componentName = entity->getComponent<ComponentName>();
            if (componentName) {
                if (componentName->name() == targetName) {
                    setTarget(entity);
                    return;
                }
            }
        }
    //}
}
void GameCamera::setPlayer(EntityWrapper* player) {
    m_Player = player;
    setState(m_State);
}
EntityWrapper* GameCamera::getTarget() {
    return m_Target; 
}
const CameraState::State GameCamera::getState() const { 
    return m_State; 
}

#pragma endregion

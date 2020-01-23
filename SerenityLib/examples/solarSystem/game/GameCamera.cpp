#include "GameCamera.h"
#include "Ship.h"
#include "map/Map.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/math/Engine_Math.h>

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

    Ship* player = dynamic_cast<Ship*>(camera.m_Player);

    switch (camera.m_State) {
        case CameraState::Cockpit: {
            if (player && player->isDestroyed())
                camera.internal_update_orbit(player, dt);
            else
                camera.internal_update_cockpit(camera.m_Player, dt);
            break;
        }case CameraState::FollowTarget: {
            if (player && player->isDestroyed())
                camera.internal_update_orbit(player, dt);
            else {
                if (camera.m_Target)
                    camera.internal_update_follow_target(camera.m_Target, dt);
                else
                    camera.internal_update_cockpit(camera.m_Player, dt);
            }
            break;
        }case CameraState::Orbit: {
            if(player && player->isDestroyed())
                camera.internal_update_orbit(player, dt);
            else
                camera.internal_update_orbit(camera.m_Target, dt);
            break;
        }case CameraState::Freeform: {
            thisCamera.lookAt(thisBody.position(), thisBody.position() + thisBody.forward(), thisBody.up());
            break;
        }
    }
} };

#pragma region GameCamera


GameCamera::GameCamera(float n, float f, Scene* scene) : GameCamera(60, Resources::getWindowSize().x / static_cast<float>(Resources::getWindowSize().y), n, f, scene) {
}
GameCamera::GameCamera(float a, float r, float n, float f,Scene* scene) : Camera(a, r, n, f, scene){
    m_State             = CameraState::Freeform;
    m_Target            = nullptr;
    m_Player            = nullptr;
    m_OrbitRadius       = 0;
    m_CameraMouseFactor = glm::dvec2(0.0);
    auto& m_Logic       = *getComponent<ComponentLogic2>();
    m_Logic.setUserPointer(this);
    m_Logic.setFunctor(GameCameraLogicFunctor());
}
GameCamera::GameCamera(float l, float r, float b, float t, float n, float f, Scene* scene) : Camera(l, r, b, t, n, f, scene){
    m_State             = CameraState::Freeform;
    m_Target            = nullptr;
    m_Player            = nullptr;
    m_OrbitRadius       = 0;
    m_CameraMouseFactor = glm::dvec2(0.0);
    auto& m_Logic       = *getComponent<ComponentLogic2>();
    m_Logic.setUserPointer(this);
    m_Logic.setFunctor(GameCameraLogicFunctor());
}
GameCamera::~GameCamera(){
}

void GameCamera::internal_update_cockpit(EntityWrapper* target, const double& dt) {
    EntityDataRequest dataRequest(m_Entity);
    auto& thisCamera = *m_Entity.getComponent<ComponentCamera>(dataRequest);
    auto& thisBody = *m_Entity.getComponent<ComponentBody>(dataRequest);

    EntityDataRequest dataRequest1(target->entity());

    auto& targetBody = *target->getComponent<ComponentBody>(dataRequest1);
    auto& targetModel = *target->getComponent<ComponentModel>(dataRequest1);
    float targetRadius = targetModel.radius();
    const auto targetFwd = glm::vec3(targetBody.forward());
    const auto targetPos = glm::vec3(targetBody.position());
    const auto targetUp = glm::vec3(targetBody.up());

    m_OrbitRadius += (static_cast<float>(Engine::getMouseWheelDelta()) * 0.007f);
    m_OrbitRadius = glm::clamp(m_OrbitRadius, 0.0f, 3.0f);

    auto calc2 = ((targetFwd * glm::length(targetRadius) * 1.7f) + targetUp * glm::length(targetRadius) * 0.3f);

    auto* ship = dynamic_cast<Ship*>(target);
    if (ship)
        calc2 += (glm::quat(targetBody.rotation()) * ship->m_CameraOffsetDefault);
    calc2 *= (1.0f + m_OrbitRadius);

    const auto pos = targetPos + calc2;

    thisBody.setPosition(pos);
    thisCamera.lookAt(pos, targetPos - (glm::vec3(50000.0f) * targetFwd), targetUp);
}
void GameCamera::internal_update_follow_target(EntityWrapper* target, const double& dt) {
    EntityDataRequest dataRequest(m_Entity);
    auto& thisCamera = *m_Entity.getComponent<ComponentCamera>(dataRequest);
    auto& thisBody = *m_Entity.getComponent<ComponentBody>(dataRequest);


    EntityDataRequest dataRequest1(target->entity());
    EntityDataRequest dataRequest2(m_Player->entity());

    auto& targetBody = *target->getComponent<ComponentBody>(dataRequest1);
    auto& player = *m_Player->getComponent<ComponentBody>(dataRequest2);
    auto& playerModel = *m_Player->getComponent<ComponentModel>(dataRequest2);

    m_OrbitRadius += (static_cast<float>(Engine::getMouseWheelDelta()) * 0.02f);
    m_OrbitRadius = glm::clamp(m_OrbitRadius, 0.0f, 3.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(player.position()) -
        ((glm::normalize(glm::vec3(targetBody.position()) - glm::vec3(player.position())) * (playerModel.radius() * 2.7f) * (1.0f + m_OrbitRadius))
            - glm::vec3(player.up()) * glm::length(playerModel.radius()) * 0.55f));

    glm::vec3 pos = Math::getMatrixPosition(model);

    thisBody.setPosition(pos);

    thisCamera.lookAt(pos, targetBody.position(), player.up());
}

void GameCamera::internal_update_orbit(EntityWrapper* target, const double& dt) {
    EntityDataRequest dataRequest(m_Entity);
    auto& thisCamera = *m_Entity.getComponent<ComponentCamera>(dataRequest);
    auto& thisBody = *m_Entity.getComponent<ComponentBody>(dataRequest);

    EntityDataRequest dataRequest1(target->entity());

    auto& targetBody = *target->getComponent<ComponentBody>(dataRequest1);
    auto& targetModel = *target->getComponent<ComponentModel>(dataRequest1);

    m_OrbitRadius += static_cast<float>(Engine::getMouseWheelDelta()) * static_cast<float>(dt) * 0.62f;
    m_OrbitRadius = glm::clamp(m_OrbitRadius, 0.0f, 70.0f);

    const auto& diff = Engine::getMouseDifference();
    auto x_amount = (diff.y * 0.8) * (dt * 0.1);
    auto y_amount = (-diff.x * 0.8) * (dt * 0.1);
    m_CameraMouseFactor.x += x_amount;
    m_CameraMouseFactor.x = glm::clamp(m_CameraMouseFactor.x, -0.06, 0.06);
    m_CameraMouseFactor.y += y_amount;
    m_CameraMouseFactor.y = glm::clamp(m_CameraMouseFactor.y, -0.06, 0.06);

    thisBody.rotate(m_CameraMouseFactor.x, m_CameraMouseFactor.y, 0);
    const double& step = (1.0 - dt);
    m_CameraMouseFactor *= (step * 0.997);

    const glm::vec3& pos = (glm::vec3(0, 0, 1) * glm::length(targetModel.radius()) * 0.37f) + (glm::vec3(0, 0, 1) * glm::length(targetModel.radius() * (1.0f + m_OrbitRadius)));

    glm::mat4 cameraModel = glm::mat4(1.0f);
    Ship* targetAsShip = dynamic_cast<Ship*>(target);
    glm::vec3 offset = glm::vec3(0.0f);
    if (targetAsShip) {
        offset += targetAsShip->m_CameraOffsetDefault;
    }
    cameraModel = glm::translate(cameraModel, glm::vec3(targetBody.position()  + ((thisBody.rotation() * glm_vec3(offset)))));
    cameraModel *= glm::mat4_cast(glm::quat(thisBody.rotation()));
    cameraModel = glm::translate(cameraModel, pos);

    const glm::vec3 eye = Math::getMatrixPosition(cameraModel);
    thisBody.setPosition(eye);

    thisCamera.lookAt(eye, targetBody.position() + (thisBody.rotation() * glm_vec3(offset)), thisBody.up());
}

Entity GameCamera::getObjectInCenterRay(vector<Entity>& exclusions) {
    Scene& scene = m_Entity.scene();
    Entity ret = Entity::_null;
    vector<Entity> objs;
    for (auto& data : priv::InternalScenePublicInterface::GetEntities(scene)) {
        Entity e = scene.getEntity(data);
        if (rayIntersectSphere(e)) {
            bool is_valid = true;
            for (auto& exclusion : exclusions) {
                if (e == exclusion) {
                    is_valid = false;
                    break;
                }
            }
            if (is_valid) {
                objs.push_back(e);
            }
        }
    }
    if (objs.size() == 0) return ret;
    if (objs.size() == 1) return objs[0];

    const auto camera_position = getPosition();
    decimal leastDistance = -1.0;
    for (auto& object : objs) {
        auto* body = object.getComponent<ComponentBody>();
        if (body) {
            const decimal distSquared = glm::distance2(body->position(), camera_position);
            if (leastDistance == -1 || distSquared < leastDistance) {
                leastDistance = distSquared;
                ret = object;
            }
        }
    }
    return ret;
}

Entity GameCamera::getObjectInCenterRay(Entity& exclusion){
    vector<Entity> ent;
    ent.push_back(exclusion);
    return getObjectInCenterRay(ent);
}

const bool GameCamera::validateDistanceForOrbit(Map& map) {
    auto* ship = dynamic_cast<Ship*>(m_Player);
    if (ship) {
        auto* sensors = static_cast<ShipSystemSensors*>(ship->getShipSystem(ShipSystemType::Sensors));
        auto* target = sensors->getTarget();
        if (target && target != m_Player) {
            const auto dist2 = glm::distance2(ship->getPosition(), target->getComponent<ComponentBody>()->position());
#if !defined(ENGINE_HIGH_PRECISION) || !defined(BT_USE_DOUBLE_PRECISION)
            if (dist2 < static_cast<decimal>(10000000000.0)) { //to prevent FP issues when viewing things billions of km away
#else
            if (dist2 < static_cast<decimal>(100000000000000.0)) { //to prevent FP issues when viewing things billions of km away
#endif
                map.centerSceneToObject(target->entity());
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

    auto& playerEntity = m_Player->entity();

    if (new_state == CameraState::Orbit) {
        m_CameraMouseFactor = glm::dvec2(0.0);

        if (old_state == CameraState::Cockpit || old_state == CameraState::FollowTarget) {
            validateDistanceForOrbit(map);
            map.centerSceneToObject(playerEntity);
        }else{ //old state is camera orbit too
            if (!m_Target || (m_Target && m_Target == m_Player)) {
                validateDistanceForOrbit(map);
            }else if(m_Player && m_Target && (m_Target != m_Player)){
                //map.centerSceneToObject(playerEntity);
                m_Target = m_Player;
            }
        }

    }else if (new_state == CameraState::Cockpit) {
        if(old_state != CameraState::Cockpit)
            map.centerSceneToObject(playerEntity);
    }else if (new_state == CameraState::FollowTarget) {
        auto* ship = dynamic_cast<Ship*>(m_Player);
        if (ship) {
            auto* sensors = static_cast<ShipSystemSensors*>(ship->getShipSystem(ShipSystemType::Sensors));
            auto* target = sensors->getTarget();
            if (m_Target != target) {
                map.centerSceneToObject(playerEntity);
                m_Target = target;
            }
        }
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

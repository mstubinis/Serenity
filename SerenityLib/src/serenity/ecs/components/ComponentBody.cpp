#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/systems/SystemBodyParentChild.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>

#pragma region Component

ComponentBody::ComponentBody(Entity entity) {
    m_Owner    = entity;
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
ComponentBody::ComponentBody(ComponentBody&& other) noexcept 
    : m_Position{ std::move(other.m_Position) }
    , m_Rotation{ std::move(other.m_Rotation) }
    , m_Scale{ std::move(other.m_Scale) }
    //, m_LinearVelocity{ std::move(other.m_LinearVelocity) }
    , m_Forward          { std::move(other.m_Forward) }
    , m_Right            { std::move(other.m_Right) }
    , m_Up               { std::move(other.m_Up) }
    , m_Owner            { std::move(other.m_Owner) }
    , m_UserPointer      { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1     { std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2     { std::exchange(other.m_UserPointer2, nullptr) }
{}
ComponentBody& ComponentBody::operator=(ComponentBody&& other) noexcept {
    //move assignment
    m_Position         = std::move(other.m_Position);
    m_Rotation         = std::move(other.m_Rotation);
    m_Scale            = std::move(other.m_Scale);
    //m_LinearVelocity   = std::move(other.m_LinearVelocity);
    m_Forward          = std::move(other.m_Forward);
    m_Right            = std::move(other.m_Right);
    m_Up               = std::move(other.m_Up);
    m_Owner            = std::move(other.m_Owner);
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
    return *this;
}
decimal ComponentBody::getDistance(Entity other) const {
    glm_vec3 my_position    = ComponentBody::getPosition();
    auto otherTransform     = other.getComponent<ComponentBody>();
    glm_vec3 other_position = otherTransform->getPosition();
    return glm::distance(my_position, other_position);
}
uint64_t ComponentBody::getDistanceLL(Entity other) const {
    return static_cast<uint64_t>(getDistance(other));
}
void ComponentBody::alignTo(decimal dirX, decimal dirY, decimal dirZ) {
    m_Rotation = Engine::Math::alignTo(dirX, dirY, dirZ);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentBody::alignTo(const glm_vec3& direction) {
    auto norm_dir = glm::normalize(direction);
    ComponentBody::alignTo(norm_dir.x, norm_dir.y, norm_dir.z);
}
void ComponentBody::translate(decimal x, decimal y, decimal z, bool local) {
    glm_vec3 offset(x, y, z);
    if (local) {
        offset = m_Rotation * offset;
    }
	ComponentBody::setPosition(m_Position + offset);
}
void ComponentBody::rotate(decimal pitch, decimal yaw, decimal roll, bool local) {
    Engine::Math::rotate(m_Rotation, pitch, yaw, roll);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentBody::scale(decimal x, decimal y, decimal z) {
    m_Scale.x += x;
    m_Scale.y += y;
    m_Scale.z += z;
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
void ComponentBody::setPosition(decimal x, decimal y, decimal z) {
	m_Position.x      = x;
    m_Position.y      = y;
    m_Position.z      = z;
    
    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = ecs.getSystem<SystemBodyParentChild>();
    auto entityIndex  = m_Owner.id() - 1;
    auto& localMatrix = system.m_LocalTransforms[entityIndex];
    localMatrix[3][0] = x;
    localMatrix[3][1] = y;
    localMatrix[3][2] = z;

    auto& worldMatrix = system.m_WorldTransforms[entityIndex];
    worldMatrix[3][0] = x;
    worldMatrix[3][1] = y;
    worldMatrix[3][2] = z;
}
void ComponentBody::setLocalPosition(decimal x, decimal y, decimal z) {
    m_Position.x      = x;
    m_Position.y      = y;
    m_Position.z      = z;

    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = ecs.getSystem<SystemBodyParentChild>();
    auto entityIndex  = m_Owner.id() - 1;
    auto& localMatrix = system.m_LocalTransforms[entityIndex];
    localMatrix[3][0] = x;
    localMatrix[3][1] = y;
    localMatrix[3][2] = z;
}
void ComponentBody::setRotation(decimal x, decimal y, decimal z, decimal w) {
    m_Rotation = glm::normalize(glm_quat{ w, x, y, z });
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentBody::setScale(decimal x, decimal y, decimal z) {
    m_Scale.x = x;
    m_Scale.y = y;
    m_Scale.z = z;
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
glm_vec3 ComponentBody::getPosition() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    auto& matrix = system.m_WorldTransforms[m_Owner.id() - 1];
    return Engine::Math::getMatrixPosition(matrix);
}
glm_vec3 ComponentBody::getWorldPosition() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    auto& matrix = system.m_WorldTransforms[m_Owner.id() - 1];
    return Engine::Math::getMatrixPosition(matrix);
}
glm_quat ComponentBody::getWorldRotation() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    return glm::quat_cast( system.m_WorldTransforms[m_Owner.id() - 1] * glm::mat4_cast(m_Rotation) );
}


glm::vec3 ComponentBody::getScreenCoordinates(bool clampToEdge) const {
	return Engine::Math::getScreenCoordinates(getPosition(), *m_Owner.scene()->getActiveCamera(), clampToEdge);
}
ScreenBoxCoordinates ComponentBody::getScreenBoxCoordinates(float minOffset) const {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = getPosition();
    auto model              = m_Owner.getComponent<ComponentModel>();
    auto radius             = 0.0001f;
    auto& camera            = *m_Owner.scene()->getActiveCamera();
    const auto center2DRes  = Engine::Math::getScreenCoordinates(worldPos, camera, false);
    const auto center2D     = glm::vec2{ center2DRes.x, center2DRes.y };
    if (model) {
        radius = model->radius();
    }else{
        ret.topLeft         = center2D;
        ret.topRight        = center2D;
        ret.bottomLeft      = center2D;
        ret.bottomRight     = center2D;
        ret.inBounds        = center2DRes.z;
        return ret;
    }
    auto& cam               = *Engine::Resources::getCurrentScene()->getActiveCamera();
    const auto camvectest   = cam.up();   
    const auto  testRes     = Engine::Math::getScreenCoordinates(worldPos + (camvectest * (decimal)radius), camera, false);
    const auto test         = glm::vec2{ testRes.x, testRes.y };
    const auto radius2D     = glm::max(minOffset, glm::distance(test, center2D));
    const auto yPlus        = center2D.y + radius2D;
    const auto yNeg         = center2D.y - radius2D;
    const auto xPlus        = center2D.x + radius2D;
    const auto xNeg         = center2D.x - radius2D;
    ret.topLeft             = glm::vec2{ xNeg,  yPlus };
    ret.topRight            = glm::vec2{ xPlus, yPlus };
    ret.bottomLeft          = glm::vec2{ xNeg,  yNeg };
    ret.bottomRight         = glm::vec2{ xPlus, yNeg };
    ret.inBounds            = center2DRes.z;
    return ret;
}
const glm_mat4& ComponentBody::getWorldMatrix() const noexcept {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    return system.m_WorldTransforms[m_Owner.id() - 1];
}
const glm_mat4& ComponentBody::getLocalMatrix() const noexcept {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    return system.m_LocalTransforms[m_Owner.id() - 1];
}


/*
void ComponentBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    glm_vec3 offset{ x, y, z };
    if (local) {
        offset = m_Rotation * offset;
    }
    m_LinearVelocity = offset;  
}
*/
void ComponentBody::addChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = ecs.getSystem<SystemBodyParentChild>();
        //system.insert(m_Owner.id(), child.id());
        system.addChild(m_Owner.id(), child.id());
    }
}
void ComponentBody::removeChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = ecs.getSystem<SystemBodyParentChild>();
        //system.remove(m_Owner.id(), child.id());
        system.removeChild(m_Owner.id(), child.id());
    }
}
/*
void ComponentBody::removeAllChildren() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    for (size_t i = 0; i < system.m_Order.size(); ++i) {
        auto entityID = system.m_Order[i];
        if (entityID == m_Owner.id()) {
            size_t j = i + 1;
            while (j < system.m_Order.size() && system.m_Order[j] > 0) {
                if (system.m_Parents[system.m_Order[j] - 1] == m_Owner.id()) {
                    //system.remove(m_Owner.id(), system.m_Order[j]);
                    system.removeChild(m_Owner.id(), system.m_Order[j]);
                }
                ++j;
            }
        }
    }
}
*/
bool ComponentBody::hasParent() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    return (system.m_Parents[m_Owner.id() - 1U] > 0);
}
Entity ComponentBody::getParent() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    return system.getParentEntity(m_Owner);
}
void ComponentBody::recalculateAllParentChildMatrices(Scene& scene) {
    auto& ecs    = Engine::priv::PublicScene::GetECS(scene);
    auto& system = ecs.getSystem<SystemBodyParentChild>();
    system.computeAllParentChildWorldTransforms();
}

#pragma endregion

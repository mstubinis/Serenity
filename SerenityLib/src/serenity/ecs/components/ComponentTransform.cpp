#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>

#pragma region Component

ComponentTransform::ComponentTransform(Entity entity) 
    : m_Owner{ entity }
{}
ComponentTransform::ComponentTransform(Entity entity, const glm_vec3& pos, const glm::quat& rot, const glm::vec3& scl)
    : m_Owner{ entity }
    , m_Position{ pos }
    , m_Rotation{ rot }
    , m_Scale{ scl }
{
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = ecs.getSystem<SystemTransformParentChild>();
    auto entityIndex  = entity.id();
    system.acquireMoreMemory(entityIndex);
    auto& localMatrix = system.m_LocalTransforms[entityIndex];
    auto& worldMatrix = system.m_WorldTransforms[entityIndex];
    Engine::Math::setFinalModelMatrix(localMatrix, m_Position, m_Rotation, m_Scale);
    worldMatrix = localMatrix;
}
ComponentTransform::ComponentTransform(ComponentTransform&& other) noexcept
    : m_Position         { std::move(other.m_Position) }
    , m_Rotation         { std::move(other.m_Rotation) }
    , m_Scale            { std::move(other.m_Scale) }
    //, m_LinearVelocity   { std::move(other.m_LinearVelocity) }
    , m_Forward          { std::move(other.m_Forward) }
    , m_Right            { std::move(other.m_Right) }
    , m_Up               { std::move(other.m_Up) }
    , m_UserPointer      { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1     { std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2     { std::exchange(other.m_UserPointer2, nullptr) }
    , m_Owner            { std::exchange(other.m_Owner, Entity{}) }
{
}
ComponentTransform& ComponentTransform::operator=(ComponentTransform&& other) noexcept {
    m_Position         = std::move(other.m_Position);
    m_Rotation         = std::move(other.m_Rotation);
    m_Scale            = std::move(other.m_Scale);
    //m_LinearVelocity   = std::move(other.m_LinearVelocity);
    m_Forward          = std::move(other.m_Forward);
    m_Right            = std::move(other.m_Right);
    m_Up               = std::move(other.m_Up);
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner            = std::exchange(other.m_Owner, Entity{});
    return *this;
}
decimal ComponentTransform::getDistance(Entity other) const {
    glm_vec3 my_position    = ComponentTransform::getPosition();
    auto otherTransform     = other.getComponent<ComponentTransform>();
    glm_vec3 other_position = otherTransform->getPosition();
    return glm::distance(my_position, other_position);
}
uint64_t ComponentTransform::getDistanceLL(Entity other) const {
    return uint64_t(getDistance(other));
}
void ComponentTransform::alignTo(float dirX, float dirY, float dirZ) {
    m_Rotation = Engine::Math::alignTo(dirX, dirY, dirZ);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentTransform::alignTo(const glm::vec3& direction) {
    auto norm_dir = glm::normalize(direction);
    alignTo(norm_dir.x, norm_dir.y, norm_dir.z);
}
void ComponentTransform::translate(decimal x, decimal y, decimal z, bool local) {
    glm_vec3 offset{ x, y, z };
    if (local) {
        offset = m_Rotation * offset;
    }
    ComponentTransform::setPosition(m_Position + offset);
}
void ComponentTransform::rotate(float pitch, float yaw, float roll, bool local) {
    Engine::Math::rotate(m_Rotation, pitch, yaw, roll, local);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentTransform::rotatePitch(float pitch, bool local) {
    Engine::Math::rotatePitch(m_Rotation, pitch, local);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentTransform::rotateYaw(float yaw, bool local) {
    Engine::Math::rotateYaw(m_Rotation, yaw, local);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentTransform::rotateRoll(float roll, bool local) {
    Engine::Math::rotateRoll(m_Rotation, roll, local);
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentTransform::scale(float x, float y, float z) {
    m_Scale.x += x;
    m_Scale.y += y;
    m_Scale.z += z;
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
void ComponentTransform::setPosition(decimal x, decimal y, decimal z) {
	m_Position.x      = x;
    m_Position.y      = y;
    m_Position.z      = z;
    
    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = ecs.getSystem<SystemTransformParentChild>();
    auto entityIndex  = m_Owner.id();
    auto& localMatrix = system.m_LocalTransforms[entityIndex];
    localMatrix[3][0] = x;
    localMatrix[3][1] = y;
    localMatrix[3][2] = z;

    auto& worldMatrix = system.m_WorldTransforms[entityIndex];
    worldMatrix[3][0] = x;
    worldMatrix[3][1] = y;
    worldMatrix[3][2] = z;
}
void ComponentTransform::setLocalPosition(decimal x, decimal y, decimal z) {
    m_Position.x      = x;
    m_Position.y      = y;
    m_Position.z      = z;

    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = ecs.getSystem<SystemTransformParentChild>();
    auto& localMatrix = system.m_LocalTransforms[m_Owner.id()];
    localMatrix[3][0] = x;
    localMatrix[3][1] = y;
    localMatrix[3][2] = z;
}
void ComponentTransform::setRotation(float x, float y, float z, float w) {
    m_Rotation = glm::normalize(glm::quat{ w, x, y, z });
    Engine::Math::recalculateForwardRightUp(m_Rotation, m_Forward, m_Right, m_Up);
}
void ComponentTransform::setScale(float x, float y, float z) {
    m_Scale.x = x;
    m_Scale.y = y;
    m_Scale.z = z;
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
glm_vec3 ComponentTransform::getPosition() const {
    const auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    const auto& system = ecs.getSystem<SystemTransformParentChild>();
    const auto& matrix = system.m_WorldTransforms[m_Owner.id()];
    return Engine::Math::getMatrixPosition(matrix);
}
glm_vec3 ComponentTransform::getWorldPosition() const {
    return getPosition();
}
glm::quat ComponentTransform::getWorldRotation() const {
    auto& worldMatrix = getWorldMatrix();
#if defined(ENGINE_HIGH_PRECISION)
    return glm::quat_cast(glm::mat4{ worldMatrix });
#else
    return glm::quat_cast(worldMatrix);
#endif
}
glm::vec3 ComponentTransform::getScreenCoordinates(bool clampToEdge) const {
	return Engine::Math::getScreenCoordinates(getPosition(), *m_Owner.scene()->getActiveCamera(), clampToEdge);
}
ScreenBoxCoordinates ComponentTransform::getScreenBoxCoordinates(float minOffset) const {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = getPosition();
    auto model              = m_Owner.getComponent<ComponentModel>();
    auto radius             = 0.0001f;
    auto& camera            = *m_Owner.scene()->getActiveCamera();
    const auto center2DRes  = Engine::Math::getScreenCoordinates(worldPos, camera, false);
    const auto center2D     = glm::vec2{ center2DRes.x, center2DRes.y };
    if (model) {
        radius = model->getRadius();
    } else {
        ret.topLeft         = center2D;
        ret.topRight        = center2D;
        ret.bottomLeft      = center2D;
        ret.bottomRight     = center2D;
        ret.inBounds        = center2DRes.z;
        return ret;
    }
    auto& cam               = *Engine::Resources::getCurrentScene()->getActiveCamera();
    const auto camvectest   = cam.getComponent<ComponentTransform>()->getUp();
    const auto testRes      = Engine::Math::getScreenCoordinates(worldPos + (camvectest * radius), camera, false);
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
const glm_mat4& ComponentTransform::getWorldMatrix() const noexcept {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.m_WorldTransforms[m_Owner.id()];
}
const glm_mat4& ComponentTransform::getLocalMatrix() const noexcept {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.m_LocalTransforms[m_Owner.id()];
}
/*
void ComponentTransform::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    glm_vec3 offset{ x, y, z };
    if (local) {
        offset = m_Rotation * offset;
    }
    m_LinearVelocity = offset;  
}
*/
void ComponentTransform::addChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = ecs.getSystem<SystemTransformParentChild>();
        system.addChild(m_Owner.id(), child.id());
    }
}
void ComponentTransform::removeChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = ecs.getSystem<SystemTransformParentChild>();
        system.removeChild(m_Owner.id(), child.id());
    }
}
bool ComponentTransform::hasParent() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.m_Parents[m_Owner.id()] != SystemTransformParentChild::NULL_IDX;
}
Entity ComponentTransform::getParent() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.getParentEntity(m_Owner);
}
void ComponentTransform::recalculateAllParentChildMatrices(Scene& scene) {
    auto& ecs    = Engine::priv::PublicScene::GetECS(scene);
    auto& system = ecs.getSystem<SystemTransformParentChild>();
    system.computeAllParentChildWorldTransforms();
}

#pragma endregion
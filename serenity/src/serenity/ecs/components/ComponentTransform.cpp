#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>


class ComponentTransform::Impl {
public:
    static void internal_recalc_direction_vectors(ComponentTransform& transform) noexcept {
#ifdef COMPONENT_TRANSFORM_STORE_RIGHT
        Engine::Math::recalculateForwardRightUp(transform.m_Rotation, transform.m_Forward, transform.m_Right, transform.m_Up);
#else
        Engine::Math::recalculateForwardUp(transform.m_Rotation, transform.m_Forward, transform.m_Up);
#endif
    }
};

#pragma region Component

ComponentTransform::ComponentTransform(Entity entity, const glm_vec3& pos, const glm::quat& rot, const glm::vec3& scl)
    : m_Position{ pos }
    , m_Rotation{ rot }
    , m_Scale{ scl }
    , m_Owner{ entity }
{
    auto& ecs        = Engine::priv::PublicScene::GetECS(m_Owner);
    auto& system     = ecs.getSystem<SystemTransformParentChild>();
    auto entityIndex = entity.id();
    system.acquireMoreMemory(entityIndex);
    Impl::internal_recalc_direction_vectors(*this);
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
#ifdef COMPONENT_TRANSFORM_STORE_RIGHT
    , m_Right            { std::move(other.m_Right) }
#endif
    , m_Up               { std::move(other.m_Up) }
    , m_Owner            { std::exchange(other.m_Owner, Entity{}) }
{}
ComponentTransform& ComponentTransform::operator=(ComponentTransform&& other) noexcept {
    if (this != &other) {
        m_Position       = std::move(other.m_Position);
        m_Rotation       = std::move(other.m_Rotation);
        m_Scale          = std::move(other.m_Scale);
        //m_LinearVelocity = std::move(other.m_LinearVelocity);
        m_Forward        = std::move(other.m_Forward);
#ifdef COMPONENT_TRANSFORM_STORE_RIGHT
        m_Right          = std::move(other.m_Right);
#endif
        m_Up             = std::move(other.m_Up);
        m_Owner          = std::exchange(other.m_Owner, Entity{});
    }
    return *this;
}
decimal ComponentTransform::getDistance(Entity other) const {
    const glm_vec3 my_position    = ComponentTransform::getPosition();
    const auto otherTransform     = other.getComponent<ComponentTransform>();
    const glm_vec3 other_position = otherTransform->getPosition();
    return glm::distance(my_position, other_position);
}
uint64_t ComponentTransform::getDistanceLL(Entity other) const {
    return uint64_t(getDistance(other));
}




void ComponentTransform::alignTo(float dirX, float dirY, float dirZ) {
    alignToDirection(dirX, dirY, dirZ);
}
void ComponentTransform::alignTo(const glm::vec3& direction) {
    alignToDirection(direction);
}
void ComponentTransform::alignToDirection(float dirX, float dirY, float dirZ) {
    m_Rotation = Engine::Math::alignTo(dirX, dirY, dirZ);
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::alignToDirection(const glm::vec3& direction) {
    alignToDirection(direction.x, direction.y, direction.z);
}
void ComponentTransform::alignToPosition(decimal x, decimal y, decimal z) {
    const glm_vec3 wPos = getWorldPosition();
    alignToDirection(float(x - wPos.x), float(y - wPos.y), float(z - wPos.z));
}
void ComponentTransform::alignToPosition(const glm_vec3& position) {
    alignToPosition(position.x, position.y, position.z);
}







void ComponentTransform::alignTo(float dirX, float dirY, float dirZ, float radPerSec) {
    alignToDirection(dirX, dirY, dirZ, radPerSec);
}
void ComponentTransform::alignTo(const glm::vec3& direction, float radPerSec) {
    alignToDirection(direction, radPerSec);
}
void ComponentTransform::alignToDirection(float dirX, float dirY, float dirZ, float radPerSec) {
    const auto degInRadians = glm::abs(Engine::Math::getAngleBetweenTwoVectors(m_Forward, glm::vec3{ dirX, dirY, dirZ }, false));
    const auto radPerStep   = Engine::Resources::dtSimulation() * radPerSec;
    const auto percentage   = std::min(radPerStep / degInRadians, 1.0f);
    const auto goal         = Engine::Math::alignTo(dirX, dirY, dirZ);
    assert(percentage >= 0.0f && percentage <= 1.0f);
    m_Rotation              = glm::normalize(glm::slerp(m_Rotation, goal, percentage));
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::alignToDirection(const glm::vec3& direction, float radPerSec) {
    alignToDirection(direction.x, direction.y, direction.z, radPerSec);
}
void ComponentTransform::alignToPosition(decimal x, decimal y, decimal z, float radPerSec) {
    const glm_vec3 wPos = getWorldPosition();
    alignToDirection(float(x - wPos.x), float(y - wPos.y), float(z - wPos.z), radPerSec);
}
void ComponentTransform::alignToPosition(const glm_vec3& position, float radPerSec) {
    alignToPosition(position.x, position.y, position.z, radPerSec);
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
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::rotatePitch(float pitch, bool local) {
    Engine::Math::rotatePitch(m_Rotation, pitch, local);
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::rotateYaw(float yaw, bool local) {
    Engine::Math::rotateYaw(m_Rotation, yaw, local);
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::rotateRoll(float roll, bool local) {
    Engine::Math::rotateRoll(m_Rotation, roll, local);
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::scale(float x, float y, float z) {
    m_Scale.x += x;
    m_Scale.y += y;
    m_Scale.z += z;
    auto modelComponent = m_Owner.getComponent<ComponentModel>();
    if (modelComponent) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*modelComponent);
    }
}
void ComponentTransform::setPosition(decimal x, decimal y, decimal z) {
    m_Position.x           = x;
    m_Position.y           = y;
    m_Position.z           = z;
    
    auto& ecs              = Engine::priv::PublicScene::GetECS(m_Owner);
    auto& system           = ecs.getSystem<SystemTransformParentChild>();
    const auto entityIndex = m_Owner.id();
    Engine::Math::setMatrixPosition(system.m_LocalTransforms[entityIndex], x, y, z);
    Engine::Math::setMatrixPosition(system.m_WorldTransforms[entityIndex], x, y, z);
}
void ComponentTransform::setLocalPosition(decimal x, decimal y, decimal z) {
    m_Position.x      = x;
    m_Position.y      = y;
    m_Position.z      = z;

    auto& ecs         = Engine::priv::PublicScene::GetECS(m_Owner);
    auto& system      = ecs.getSystem<SystemTransformParentChild>();
    Engine::Math::setMatrixPosition(system.m_LocalTransforms[m_Owner.id()], x, y, z);
}
void ComponentTransform::setRotation(float x, float y, float z, float w) {
    m_Rotation = glm::normalize(glm::quat{ w, x, y, z });
    Impl::internal_recalc_direction_vectors(*this);
}
void ComponentTransform::setScale(float x, float y, float z) {
    m_Scale.x = x;
    m_Scale.y = y;
    m_Scale.z = z;
    auto modelComponent = m_Owner.getComponent<ComponentModel>();
    if (modelComponent) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*modelComponent);
    }
}
glm_vec3 ComponentTransform::getPosition() const {
    const auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
    const auto& system = ecs.getSystem<SystemTransformParentChild>();
    const auto& matrix = system.m_WorldTransforms[m_Owner.id()];
    return Engine::Math::getMatrixPosition(matrix);
}
glm_vec3 ComponentTransform::getWorldPosition() const {
    return getPosition();
}
glm::quat ComponentTransform::getWorldRotation() const {
    const auto& worldMatrix = getWorldMatrix();
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
    const auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
    const auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.m_WorldTransforms[m_Owner.id()];
}
const glm_mat4& ComponentTransform::getLocalMatrix() const noexcept {
    const auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
    const auto& system = ecs.getSystem<SystemTransformParentChild>();
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
        auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
        auto& system = ecs.getSystem<SystemTransformParentChild>();
        system.addChild(m_Owner.id(), child.id());
    }
}
void ComponentTransform::removeChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
        auto& system = ecs.getSystem<SystemTransformParentChild>();
        system.removeChild(m_Owner.id(), child.id());
    }
}
bool ComponentTransform::hasParent() const {
    const auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
    const auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.m_Parents[m_Owner.id()] != SystemTransformParentChild::NULL_IDX;
}
Entity ComponentTransform::getParent() const {
    const auto& ecs    = Engine::priv::PublicScene::GetECS(m_Owner);
    const auto& system = ecs.getSystem<SystemTransformParentChild>();
    return system.getParentEntity(m_Owner);
}


void ComponentTransform::recalculateAllParentChildMatrices(Scene& scene) {
    auto& ecs    = Engine::priv::PublicScene::GetECS(scene);
    auto& system = ecs.getSystem<SystemTransformParentChild>();
    system.computeAllParentChildWorldTransforms();
}

#pragma endregion

#pragma region ComponentTransformLUABinder

void Engine::priv::ComponentTransformLUABinder::addChild(Entity child) const {
    m_Owner.getComponent<ComponentTransform>()->addChild(child);
}
void Engine::priv::ComponentTransformLUABinder::removeChild(Entity child) const {
    m_Owner.getComponent<ComponentTransform>()->removeChild(child);
}
bool Engine::priv::ComponentTransformLUABinder::hasParent() const {
    return m_Owner.getComponent<ComponentTransform>()->hasParent();
}

void Engine::priv::ComponentTransformLUABinder::setPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->setPosition(x.cast<decimal>(), y.cast<decimal>(), z.cast<decimal>());
        } else if(!x.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->setPosition(x.cast<glm_vec3>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->setPosition(x.cast<decimal>());
        }
    }
}
void Engine::priv::ComponentTransformLUABinder::setRotation(float x, float y, float z, float w) const {
    m_Owner.getComponent<ComponentTransform>()->setRotation(x, y, z, w);
}
void Engine::priv::ComponentTransformLUABinder::setScale(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->setScale(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else if (!x.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->setScale(x.cast<glm::vec3>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->setScale(x.cast<float>());
        }
    }
}

void Engine::priv::ComponentTransformLUABinder::setLocalPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->setLocalPosition(x.cast<decimal>(), y.cast<decimal>(), z.cast<decimal>());
        } else if(!x.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->setLocalPosition(x.cast<glm_vec3>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->setLocalPosition(x.cast<decimal>());
        }
    }
}

void Engine::priv::ComponentTransformLUABinder::translate(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z, bool local) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->translate(x.cast<decimal>(), y.cast<decimal>(), z.cast<decimal>(), local);
        } else if(!x.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->translate(x.cast<glm_vec3>(), local);
        } else {
            m_Owner.getComponent<ComponentTransform>()->translate(x.cast<decimal>(), local);
        }
    }
}
void Engine::priv::ComponentTransformLUABinder::rotate(float x, float y, float z, bool local) const {
    m_Owner.getComponent<ComponentTransform>()->rotate(x, y, z, local);
}
void Engine::priv::ComponentTransformLUABinder::scale(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->scale(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else if (!x.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->scale(x.cast<glm::vec3>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->scale(x.cast<float>());
        }
    }
}
decimal Engine::priv::ComponentTransformLUABinder::getDistance(Entity other) const {
    return m_Owner.getComponent<ComponentTransform>()->getDistance(other);
}

glm_vec3 Engine::priv::ComponentTransformLUABinder::getPosition() const {
    return m_Owner.getComponent<ComponentTransform>()->getPosition();
}
glm::quat Engine::priv::ComponentTransformLUABinder::getRotation() const {
    return m_Owner.getComponent<ComponentTransform>()->getRotation();
}
glm::vec3 Engine::priv::ComponentTransformLUABinder::getScale() const {
    return m_Owner.getComponent<ComponentTransform>()->getScale();
}

glm_vec3 Engine::priv::ComponentTransformLUABinder::getLocalPosition() const {
    return m_Owner.getComponent<ComponentTransform>()->getLocalPosition();
}

//TODO: use const references?
glm::vec3 Engine::priv::ComponentTransformLUABinder::getForward() const {
    return m_Owner.getComponent<ComponentTransform>()->getForward();
}
glm::vec3 Engine::priv::ComponentTransformLUABinder::getRight() const {
    return m_Owner.getComponent<ComponentTransform>()->getRight();
}
glm::vec3 Engine::priv::ComponentTransformLUABinder::getUp() const {
    return m_Owner.getComponent<ComponentTransform>()->getUp();
}

void Engine::priv::ComponentTransformLUABinder::alignTo(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->alignTo(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->alignTo(x.cast<glm::vec3>());
        }
    }
}
void Engine::priv::ComponentTransformLUABinder::alignToDirection(luabridge::LuaRef dirX, luabridge::LuaRef dirY, luabridge::LuaRef dirZ) {
    if (!dirX.isNil()) {
        if (dirX.isNumber() && dirY.isNumber() && dirZ.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->alignToDirection(dirX.cast<float>(), dirY.cast<float>(), dirZ.cast<float>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->alignToDirection(dirX.cast<glm::vec3>());
        }
    }
}
void Engine::priv::ComponentTransformLUABinder::alignToPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_Owner.getComponent<ComponentTransform>()->alignToPosition(x.cast<decimal>(), y.cast<decimal>(), z.cast<decimal>());
        } else {
            m_Owner.getComponent<ComponentTransform>()->alignToPosition(x.cast<glm_vec3>());
        }
    }
}
const glm_mat4& Engine::priv::ComponentTransformLUABinder::getWorldMatrix() const {
    return m_Owner.getComponent<ComponentTransform>()->getWorldMatrix();
}
glm::mat4 Engine::priv::ComponentTransformLUABinder::getWorldMatrixRendering() const {
    return m_Owner.getComponent<ComponentTransform>()->getWorldMatrixRendering();
}
const glm_mat4& Engine::priv::ComponentTransformLUABinder::getLocalMatrix() const {
    return m_Owner.getComponent<ComponentTransform>()->getLocalMatrix();
}


#pragma endregion
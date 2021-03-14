#include <serenity/ecs/components/ComponentBodyRigid.h>

#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/physics/PhysicsModule.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/physics/Collision.h>
#include <serenity/scene/Scene.h>
#include <serenity/ecs/systems/SystemComponentBody.h>
#include <serenity/ecs/systems/SystemBodyParentChild.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#pragma region ComponentBodyRigid

ComponentBodyRigid::ComponentBodyRigid(Entity entity, CollisionType collisionType) {
    m_Owner = entity;
    setCollision(collisionType, 1.0f);
    rebuildRigidBody(false);
}
ComponentBodyRigid::~ComponentBodyRigid() {
    Engine::Physics::removeRigidBody(m_BulletRigidBody.get());
}
ComponentBodyRigid::ComponentBodyRigid(ComponentBodyRigid&& other) noexcept
    : m_Forward           { std::move(other.m_Forward) }
    , m_Right             { std::move(other.m_Right) }
    , m_Up                { std::move(other.m_Up) }
    , m_CollisionFunctor  { std::move(other.m_CollisionFunctor) }
    , m_Collision         { std::move(other.m_Collision) }
    , m_BulletRigidBody   { std::move(other.m_BulletRigidBody) }
    , m_BulletMotionState { std::move(other.m_BulletMotionState) }
    , m_UserPointer       { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1      { std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2      { std::exchange(other.m_UserPointer2, nullptr) }
    , m_Owner             { std::exchange(other.m_Owner, Entity{}) }
    , m_Mass              { std::move(other.m_Mass) }
    , m_Group             { std::move(other.m_Group) }
    , m_Mask              { std::move(other.m_Mask) }
    , m_ForcedOut         { std::move(other.m_ForcedOut) }
{
    setInternalPhysicsUserPointer(this);
}
ComponentBodyRigid& ComponentBodyRigid::operator=(ComponentBodyRigid&& other) noexcept {
    ASSERT(&other != this, __FUNCTION__ << "(): trying to move itself!");
    m_Forward            = std::move(other.m_Forward);
    m_Right              = std::move(other.m_Right);
    m_Up                 = std::move(other.m_Up);
    m_CollisionFunctor   = std::move(other.m_CollisionFunctor);
    m_Collision          = std::move(other.m_Collision);
    m_BulletMotionState  = std::move(other.m_BulletMotionState);
    m_UserPointer        = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1       = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2       = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner              = std::exchange(other.m_Owner, Entity{});
    m_Mass               = std::move(other.m_Mass);
    m_Group              = std::move(other.m_Group);
    m_Mask               = std::move(other.m_Mask);
    m_ForcedOut          = std::move(other.m_ForcedOut);

    if (m_BulletRigidBody) {
        Engine::Physics::removeRigidBody(m_BulletRigidBody.get());
    }
    m_BulletRigidBody = std::move(other.m_BulletRigidBody);
    if (m_BulletRigidBody) {
        m_BulletRigidBody->setCollisionShape(m_Collision->getBtShape());
    }

    setInternalPhysicsUserPointer(this);
    return *this;
}
void ComponentBodyRigid::rebuildRigidBody(bool addBodyToPhysicsWorld, bool threadSafe) {
    auto& inertia = m_Collision->getBtInertia();
    auto* shape   = m_Collision->getBtShape();
    if (shape) {
        if (m_BulletRigidBody) {
            removePhysicsFromWorld(true, threadSafe);
        }
        m_BulletRigidBody.reset(new btRigidBody{ btRigidBody::btRigidBodyConstructionInfo{ m_Mass, &m_BulletMotionState, shape, inertia } });
        m_BulletRigidBody->setSleepingThresholds(0.015f, 0.015f);
        m_BulletRigidBody->setFriction(0.3f);
        setDamping((decimal)(0.1), (decimal)(0.4));
        m_BulletRigidBody->setMassProps((btScalar)(m_Mass), inertia);
        m_BulletRigidBody->updateInertiaTensor();
        setInternalPhysicsUserPointer(this);
        if (addBodyToPhysicsWorld) {
            addPhysicsToWorld(true, threadSafe);
        }
    }
}
//kinda ugly
void ComponentBodyRigid::setInternalPhysicsUserPointer(void* userPtr) {
    if (m_BulletRigidBody) {
        m_BulletRigidBody->setUserPointer(userPtr);
        auto* shape = m_BulletRigidBody->getCollisionShape();
        if (shape) {
            shape->setUserPointer(userPtr);
        }
    }
}
void ComponentBodyRigid::removePhysicsFromWorld(bool force, bool threadSafe) {
    if (force) {
        m_ForcedOut = true;
    }
    (threadSafe) ? Engine::Physics::removeRigidBodyThreadSafe(*this) : Engine::Physics::removeRigidBody(*this);
}
void ComponentBodyRigid::addPhysicsToWorld(bool force, bool threadSafe) {
    if (!force && m_ForcedOut) {
        return;
    }
    (threadSafe) ? Engine::Physics::addRigidBodyThreadSafe(*this) : Engine::Physics::addRigidBody(*this);
    m_ForcedOut = false;
}
decimal ComponentBodyRigid::getLinearDamping() const {
    return (m_BulletRigidBody) ? (decimal)m_BulletRigidBody->getLinearDamping() : decimal(0.0);
}
decimal ComponentBodyRigid::getAngularDamping() const {
    return (m_BulletRigidBody) ? (decimal)m_BulletRigidBody->getAngularDamping() : decimal(0.0);
}
void ComponentBodyRigid::collisionResponse(RigidCollisionCallbackEventData& data) const {
    if (m_CollisionFunctor) { //TODO: find out why this is needed and possibly remove this if check
        m_CollisionFunctor(data);
    }
}
MaskType ComponentBodyRigid::getCollisionFlags() const {
    return static_cast<MaskType>(m_BulletRigidBody->getCollisionFlags());
}
//TODO: include ComponentBody too
decimal ComponentBodyRigid::getDistance(Entity other) const {
    glm_vec3 my_position    = ComponentBodyRigid::getPosition();
    auto otherBody          = other.getComponent<ComponentBody>();
    auto otherRigid         = other.getComponent<ComponentBodyRigid>();
    glm_vec3 other_position = otherBody ? otherBody->getPosition() : otherRigid->getPosition();
    return glm::distance(my_position, other_position);
}
uint64_t ComponentBodyRigid::getDistanceLL(Entity other) const {
    return static_cast<uint64_t>(getDistance(other));
}
void ComponentBodyRigid::alignTo(decimal dirX, decimal dirY, decimal dirZ) {
    //recheck this
    auto q = Engine::Math::alignTo(dirX, dirY, dirZ);
    ComponentBodyRigid::setRotation(q);
}
void ComponentBodyRigid::alignTo(const glm_vec3& direction) {
    auto norm_dir = glm::normalize(direction);
    ComponentBodyRigid::alignTo(norm_dir.x, norm_dir.y, norm_dir.z);
}
void ComponentBodyRigid::internal_update_misc() noexcept {
    if (m_BulletRigidBody) {
        m_BulletRigidBody->setCollisionShape(m_Collision->getBtShape());
        m_BulletRigidBody->setMassProps((btScalar)m_Mass, m_Collision->getBtInertia());
        m_BulletRigidBody->updateInertiaTensor();
    }
}
void ComponentBodyRigid::setCollision(CollisionType collisionType, float mass) {
    if (!m_Collision) { //TODO: clean this up, its hacky and evil. its being used on the ComponentBody_EntityAddedToSceneFunction
        auto modelComponent = m_Owner.getComponent<ComponentModel>();
        if (modelComponent) {
            if (collisionType == CollisionType::Compound) {
                m_Collision = std::make_unique<Collision>(*this, *modelComponent, mass);
            } else {
                m_Collision = std::make_unique<Collision>(*this, collisionType, &modelComponent->getModel(), mass);
            }
        } else {
            m_Collision = std::make_unique<Collision>(*this, collisionType, nullptr, mass);
        }
    }
    m_Mass = mass;
    m_Collision->setMass(m_Mass);
    internal_update_misc();
    setInternalPhysicsUserPointer(this);
}
void ComponentBodyRigid::translate(decimal x, decimal y, decimal z, bool local) {
    btVector3 vec = internal_activate_and_get_vector(x, y, z, local);
    ComponentBodyRigid::setPosition(getPosition() + Engine::Math::btVectorToGLM(vec));  
}
void ComponentBodyRigid::rotate(decimal pitch, decimal yaw, decimal roll, bool local) {
    auto& bt_rigidBody  = *m_BulletRigidBody;
    btQuaternion quat   = bt_rigidBody.getWorldTransform().getRotation().normalize();
    glm_quat glmquat{ quat.w(), quat.x(), quat.y(), quat.z() };
    Engine::Math::rotate(glmquat, pitch, yaw, roll);
    quat                = btQuaternion{ (btScalar)glmquat.x, (btScalar)glmquat.y, (btScalar)glmquat.z, (btScalar)glmquat.w };
    bt_rigidBody.getWorldTransform().setRotation(quat);
    Engine::Math::recalculateForwardRightUp(bt_rigidBody, m_Forward, m_Right, m_Up);
}
void ComponentBodyRigid::scale(decimal x, decimal y, decimal z) {
    const auto newScale = btVector3{ (btScalar)x, (btScalar)y, (btScalar)z };
    auto collisionShape = m_Collision->getBtShape();
    if (collisionShape) {
        collisionShape->setLocalScaling(collisionShape->getLocalScaling() + newScale);
    }
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
void ComponentBodyRigid::setPosition(decimal x, decimal y, decimal z) {
    btTransform tr;
    tr.setOrigin(btVector3{ (btScalar)x, (btScalar)y, (btScalar)z });
    tr.setRotation(m_BulletRigidBody->getOrientation());
    if (m_Collision->getType() == CollisionType::TriangleShapeStatic) {
        //removePhysicsFromWorld(false, false);
        m_BulletRigidBody->setCollisionShape(m_Collision->getBtShape());
        removePhysicsFromWorld(false, true);
    }
    m_BulletMotionState.setWorldTransform(tr);
    m_BulletRigidBody->setMotionState(&m_BulletMotionState); //is this needed?
    m_BulletRigidBody->setWorldTransform(tr);
    m_BulletRigidBody->setCenterOfMassTransform(tr);
    if (m_Collision->getType() == CollisionType::TriangleShapeStatic) {
        //addPhysicsToWorld(false, false);
        addPhysicsToWorld(false, true);
    }

    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = (SystemBodyParentChild&)ecs.getSystem<SystemBodyParentChild>();
    auto entityIndex  = m_Owner.id() - 1U;
    auto& localMatrix = system.LocalTransforms[entityIndex];
    localMatrix[3][0] = x;
    localMatrix[3][1] = y;
    localMatrix[3][2] = z;

    auto& worldMatrix = system.WorldTransforms[entityIndex];
    worldMatrix[3][0] = x;
    worldMatrix[3][1] = y;
    worldMatrix[3][2] = z;
}
void ComponentBodyRigid::setGravity(decimal x, decimal y, decimal z) {
    m_BulletRigidBody->setGravity(btVector3{ (btScalar)x, (btScalar)y, (btScalar)z });
}
void ComponentBodyRigid::setRotation(decimal x, decimal y, decimal z, decimal w) {
    btQuaternion quat{ (btScalar)x, (btScalar)y, (btScalar)z, (btScalar)w };
    quat = quat.normalize();
    auto& rigidBody = *m_BulletRigidBody;
    btTransform tr;
    tr.setOrigin(rigidBody.getWorldTransform().getOrigin());
    tr.setRotation(quat);
    rigidBody.setWorldTransform(tr);
    rigidBody.setCenterOfMassTransform(tr);
    m_BulletMotionState.setWorldTransform(tr);
    Engine::Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
    clearAngularForces();
}
void ComponentBodyRigid::setScale(decimal x, decimal y, decimal z) {
    const auto newScale = btVector3{ (btScalar)x, (btScalar)y, (btScalar)z };
    auto collisionShape = m_Collision->getBtShape();
    if (collisionShape) {
        collisionShape->setLocalScaling(newScale);
    }
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
glm_vec3 ComponentBodyRigid::getLocalPosition() const { //theres prob a better way to do this
    //btTransform tr;
    //m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    auto& tr = m_BulletRigidBody->getWorldTransform();
    return Engine::Math::btVectorToGLM(tr.getOrigin());
}
glm_vec3 ComponentBodyRigid::getPosition() const { //theres prob a better way to do this
    //btTransform tr;
    //m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    auto& tr = m_BulletRigidBody->getWorldTransform();
    return Engine::Math::btVectorToGLM(tr.getOrigin());
}
glm::vec3 ComponentBodyRigid::getScreenCoordinates(bool clampToEdge) const {
    return Engine::Math::getScreenCoordinates(getPosition(), *m_Owner.scene()->getActiveCamera(), clampToEdge);
}
ScreenBoxCoordinates ComponentBodyRigid::getScreenBoxCoordinates(float minOffset) const {
    ScreenBoxCoordinates ret;
    const auto& worldPos   = getPosition();
    auto model             = m_Owner.getComponent<ComponentModel>();
    auto radius            = 0.0001f;
    auto& camera           = *m_Owner.scene()->getActiveCamera();
    const auto center2DRes = Engine::Math::getScreenCoordinates(worldPos, camera, false);
    const auto center2D    = glm::vec2{ center2DRes.x, center2DRes.y };
    if (model) {
        radius = model->radius();
    }else{
        ret.topLeft     = center2D;
        ret.topRight    = center2D;
        ret.bottomLeft  = center2D;
        ret.bottomRight = center2D;
        ret.inBounds    = center2DRes.z;
        return ret;
    }
    auto& cam             = *Engine::Resources::getCurrentScene()->getActiveCamera();
    const auto camvectest = cam.up();
    const auto  testRes   = Engine::Math::getScreenCoordinates(worldPos + (camvectest * (decimal)radius), camera, false);
    const auto test       = glm::vec2{ testRes.x, testRes.y };
    const auto radius2D   = glm::max(minOffset, glm::distance(test, center2D));
    const auto yPlus      = center2D.y + radius2D;
    const auto yNeg       = center2D.y - radius2D;
    const auto xPlus      = center2D.x + radius2D;
    const auto xNeg       = center2D.x - radius2D;
    ret.topLeft           = glm::vec2{ xNeg,  yPlus };
    ret.topRight          = glm::vec2{ xPlus, yPlus };
    ret.bottomLeft        = glm::vec2{ xNeg,  yNeg };
    ret.bottomRight       = glm::vec2{ xPlus, yNeg };
    ret.inBounds          = center2DRes.z;
    return ret;
}
glm_vec3 ComponentBodyRigid::getScale() const {
    auto collisionShape = m_Collision->getBtShape();
    if (collisionShape) {
        return Engine::Math::btVectorToGLM(collisionShape->getLocalScaling());
    }
    return glm_vec3((decimal)1.0);
}
glm_quat ComponentBodyRigid::getRotation() const {
    //btTransform tr;
    //m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    auto& tr = m_BulletRigidBody->getWorldTransform();
    return Engine::Math::btToGLMQuat(tr.getRotation());
}
glm_vec3 ComponentBodyRigid::getLinearVelocity() const {
    return Engine::Math::btVectorToGLM(m_BulletRigidBody->getLinearVelocity());
}
glm_vec3 ComponentBodyRigid::getAngularVelocity() const {
    return Engine::Math::btVectorToGLM(m_BulletRigidBody->getAngularVelocity());
}
glm_mat4 ComponentBodyRigid::modelMatrix() const { //theres prob a better way to do this
    //btTransform tr;
    //m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    auto& tr = m_BulletRigidBody->getWorldTransform();
    glm_mat4 outModelMatrix{ (decimal)1.0 };
    tr.getOpenGLMatrix(glm::value_ptr(outModelMatrix));
    if (m_Collision->getBtShape()) {
        auto scale     = ComponentBodyRigid::getScale();
        outModelMatrix = glm::scale(outModelMatrix, scale);
    }
    return outModelMatrix;
}
void ComponentBodyRigid::setDamping(decimal linearFactor, decimal angularFactor) {
    m_BulletRigidBody->setDamping((btScalar)linearFactor, (btScalar)angularFactor);
}
void ComponentBodyRigid::setCollisionGroup(MaskType group) {
    removePhysicsFromWorld(true);
    m_Group = group;
    addPhysicsToWorld(true);
}
void ComponentBodyRigid::setCollisionMask(MaskType mask) {
    removePhysicsFromWorld(true);
    m_Mask = mask;
    addPhysicsToWorld(true);
}
void ComponentBodyRigid::addCollisionGroup(MaskType group) {
    removePhysicsFromWorld(true);
    m_Group |= group;
    addPhysicsToWorld(true);
}
void ComponentBodyRigid::addCollisionMask(MaskType mask) {
    removePhysicsFromWorld(true);
    m_Mask |= mask;
    addPhysicsToWorld(true);
}
void ComponentBodyRigid::setCollisionFlag(MaskType flag) {
    auto& BtRigidBody = *m_BulletRigidBody;
    auto currFlags = BtRigidBody.getCollisionFlags();
    if (currFlags != flag) {
        removePhysicsFromWorld(true);
        BtRigidBody.setCollisionFlags(flag);
        addPhysicsToWorld(true);
    }
}
void ComponentBodyRigid::addCollisionFlag(MaskType flag) {
    auto& BtRigidBody = *m_BulletRigidBody;
    auto currFlags = BtRigidBody.getCollisionFlags();
    auto added_flags = (currFlags | flag);
    if (currFlags != added_flags) {
        removePhysicsFromWorld(true);
        BtRigidBody.setCollisionFlags(added_flags);
        addPhysicsToWorld(true);
    }
}
void ComponentBodyRigid::removeCollisionGroup(MaskType group) {
    removePhysicsFromWorld(true);
    m_Group &= ~group;
    addPhysicsToWorld(true);
}
void ComponentBodyRigid::removeCollisionMask(MaskType mask) {
    removePhysicsFromWorld(true);
    m_Mask &= ~mask;
    addPhysicsToWorld(true);
}
void ComponentBodyRigid::removeCollisionFlag(MaskType flag) {
    auto& BtRigidBody = *m_BulletRigidBody;
    auto currFlags = BtRigidBody.getCollisionFlags();
    auto removed_flags = (currFlags & ~flag);
    if (currFlags != removed_flags) {
        removePhysicsFromWorld(true);
        BtRigidBody.setCollisionFlags(removed_flags);
        addPhysicsToWorld(true);
    }
}
//TODO: reconsider how this works
void ComponentBodyRigid::setDynamic(bool dynamic) {
    if (dynamic) {
        m_BulletRigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
        m_BulletRigidBody->activate();
    }else{
        m_BulletRigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        ComponentBodyRigid::clearAllForces();
        m_BulletRigidBody->activate();
    }
}
btVector3 ComponentBodyRigid::internal_activate_and_get_vector(decimal x, decimal y, decimal z, bool local) noexcept {
    m_BulletRigidBody->activate();
    btVector3 vec{ static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z) };
    Engine::Math::translate(*m_BulletRigidBody, vec, local);
    return vec;
}
void ComponentBodyRigid::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    m_BulletRigidBody->setLinearVelocity(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentBodyRigid::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    m_BulletRigidBody->setAngularVelocity(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentBodyRigid::applyForce(decimal x, decimal y, decimal z, bool local) {
    m_BulletRigidBody->applyCentralForce(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentBodyRigid::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    btVector3 v = internal_activate_and_get_vector(force.x, force.y, force.z, local);
    m_BulletRigidBody->applyForce(v, btVector3{ static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z) });
}
void ComponentBodyRigid::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    m_BulletRigidBody->applyCentralImpulse(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentBodyRigid::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    btVector3 v = internal_activate_and_get_vector(impulse.x, impulse.y, impulse.z, local);
    m_BulletRigidBody->applyImpulse(v, btVector3{ static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z) });
}
void ComponentBodyRigid::applyTorque(decimal x, decimal y, decimal z, bool local) {
    btVector3 v = internal_activate_and_get_vector(x, y, z, false); //yes, keep this false
    if (local) {
        v = m_BulletRigidBody->getInvInertiaTensorWorld().inverse() * (m_BulletRigidBody->getWorldTransform().getBasis() * v);
    }
    m_BulletRigidBody->applyTorque(v);
}
void ComponentBodyRigid::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    btVector3 v = internal_activate_and_get_vector(x, y, z, false); //yes, keep this false
    if (local) {
        v = m_BulletRigidBody->getInvInertiaTensorWorld().inverse() * (m_BulletRigidBody->getWorldTransform().getBasis() * v);
    }
    m_BulletRigidBody->applyTorqueImpulse(v);
}
void ComponentBodyRigid::clearLinearForces() {
    m_BulletRigidBody->setActivationState(0);
    m_BulletRigidBody->activate();
    m_BulletRigidBody->setLinearVelocity(btVector3{ 0, 0, 0 });
}
void ComponentBodyRigid::clearAngularForces() {
    m_BulletRigidBody->setActivationState(0);
    m_BulletRigidBody->activate();
    m_BulletRigidBody->setAngularVelocity(btVector3{ 0, 0, 0 });
}
void ComponentBodyRigid::clearAllForces() {
    clearLinearForces();
    clearAngularForces();
}
void ComponentBodyRigid::setMass(float mass) {
    m_Mass = mass;
    if (m_Collision->getBtShape()) {
        m_Collision->setMass(m_Mass);
        if (m_BulletRigidBody) {
            m_BulletRigidBody->setMassProps(static_cast<btScalar>(m_Mass), m_Collision->getBtInertia());
        }
    }
}
void ComponentBodyRigid::addChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = (SystemBodyParentChild&)ecs.getSystem<SystemBodyParentChild>();
        system.insert(m_Owner.id(), child.id());
    }
}
void ComponentBodyRigid::removeChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = (SystemBodyParentChild&)ecs.getSystem<SystemBodyParentChild>();
        system.remove(m_Owner.id(), child.id());
    }
}
void ComponentBodyRigid::removeAllChildren() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = (SystemBodyParentChild&)ecs.getSystem<SystemBodyParentChild>();
    for (size_t i = 0; i < system.Order.size(); ++i) {
        auto entityID = system.Order[i];
        if (entityID == m_Owner.id()) {
            size_t j = i + 1;
            while (j < system.Order.size() && system.Order[j] > 0) {
                if (system.Parents[system.Order[j] - 1U] == m_Owner.id()) {
                    system.remove(m_Owner.id(), system.Order[j]);
                }
                ++j;
            }
        }
    }
}
bool ComponentBodyRigid::hasParent() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = (SystemBodyParentChild&)ecs.getSystem<SystemBodyParentChild>();
    return (system.Parents[m_Owner.id() - 1U] > 0);
}
void ComponentBodyRigid::recalculateAllParentChildMatrices(Scene& scene) {
    auto& ecs    = Engine::priv::PublicScene::GetECS(scene);
    auto& system = (SystemBodyParentChild&)ecs.getSystem<SystemBodyParentChild>();
    system.computeAllMatrices();
}

#pragma endregion

#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/systems/SystemAddRigidBodies.h>
#include <serenity/ecs/systems/SystemRemoveRigidBodies.h>
#include <serenity/physics/PhysicsModule.h>
#include <serenity/scene/Scene.h>
#include <serenity/ecs/ECS.h>

#pragma region ComponentRigidBody

ComponentRigidBody::ComponentRigidBody(Entity entity) {
    m_Owner = entity;
    rebuildRigidBody(false);
}
ComponentRigidBody::~ComponentRigidBody() {
    if (m_Owner) { //do not call from moved from destructors
        auto& ecs          = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& systemRemove = ecs.getSystem<SystemRemoveRigidBodies>();
        auto& systemAdd    = ecs.getSystem<SystemAddRigidBodies>();
        auto BTRigidBody   = getBtBody();
        if (BTRigidBody) {
            systemAdd.removeBody(BTRigidBody);             // clear any deferred adding of this body
            systemRemove.removeBody(BTRigidBody);          // clear any deferred removal of this body
            bool result = Engine::Physics::removeRigidBody(BTRigidBody); // remove it immediately 
        }
    }
}
ComponentRigidBody::ComponentRigidBody(ComponentRigidBody&& other) noexcept
    : m_CollisionFunctor  { std::move(other.m_CollisionFunctor) }
    , m_BulletRigidBody   { std::move(other.m_BulletRigidBody) }
    , m_BulletMotionState { std::move(other.m_BulletMotionState) }
    , m_UserPointer       { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1      { std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2      { std::exchange(other.m_UserPointer2, nullptr) }
    , m_Mass              { std::move(other.m_Mass) }
    , m_Owner             { std::exchange(other.m_Owner, Entity{}) }
    , m_Group             { std::move(other.m_Group) }
    , m_Mask              { std::move(other.m_Mask) }
{
    internal_update_misc();
}
ComponentRigidBody& ComponentRigidBody::operator=(ComponentRigidBody&& other) noexcept {
    m_CollisionFunctor  = std::move(other.m_CollisionFunctor);
    m_BulletRigidBody   = std::move(other.m_BulletRigidBody);
    m_BulletMotionState = std::move(other.m_BulletMotionState);
    m_UserPointer       = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1      = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2      = std::exchange(other.m_UserPointer2, nullptr);
    m_Mass              = std::move(other.m_Mass);
    m_Owner             = std::exchange(other.m_Owner, Entity{});
    m_Group             = std::move(other.m_Group);
    m_Mask              = std::move(other.m_Mask);

    internal_update_misc();
    return *this;
}
void ComponentRigidBody::internal_update_misc() noexcept {
    if (m_BulletRigidBody) {
        m_BulletRigidBody->setMotionState(&m_BulletMotionState);
        auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
        if (collisionShape) {
            m_BulletRigidBody->setCollisionShape(collisionShape->getBtShape());
            m_BulletRigidBody->setMassProps(m_Mass, collisionShape->getInertia());
        }
        m_BulletRigidBody->updateInertiaTensor();
        m_BulletRigidBody->setUserPointer(this);
    }
}
bool ComponentRigidBody::rebuildRigidBody(bool addBodyToPhysicsWorld) {
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    auto transform      = m_Owner.getComponent<ComponentTransform>();
    ASSERT(collisionShape, __FUNCTION__ << "(): collisionShape was nullptr!");
    if (m_BulletRigidBody) {
        removePhysicsFromWorld();
    }

    btRigidBody::btRigidBodyConstructionInfo ci{ m_Mass, &m_BulletMotionState, collisionShape->getBtShape(), collisionShape->getInertia() };
    if (transform) {
        btTransform t;
        t.setOrigin(Engine::Math::toBT(transform->getWorldPosition()));
        t.setRotation(Engine::Math::toBT(transform->getWorldRotation()));
        ci.m_startWorldTransform  = t;
    }
    ci.m_linearDamping            = (btScalar)0.1;
    ci.m_angularDamping           = (btScalar)0.4;
    ci.m_friction                 = (btScalar)0.3;
    ci.m_linearSleepingThreshold  = (btScalar)0.015;
    ci.m_angularSleepingThreshold = (btScalar)0.015;

    m_BulletRigidBody.reset(new btRigidBodyType { ci });
    m_BulletRigidBody->setMassProps(m_Mass, collisionShape->getInertia());
    m_BulletRigidBody->updateInertiaTensor();
    m_BulletRigidBody->setUserPointer(this);
    internal_calculate_mass();
    if (addBodyToPhysicsWorld) {
        addPhysicsToWorld();
    }
    return (collisionShape != nullptr);
}
bool ComponentRigidBody::removePhysicsFromWorld() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    auto& ecs          = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& systemRemove = ecs.getSystem<SystemRemoveRigidBodies>();
    return systemRemove.enqueueBody(getBtBody());
}
bool ComponentRigidBody::addPhysicsToWorld() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    auto& ecs       = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& systemAdd = ecs.getSystem<SystemAddRigidBodies>();
    return systemAdd.enqueueBody(getBtBody(), m_Group, m_Mask);
}
decimal ComponentRigidBody::getLinearDamping() const {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    return static_cast<decimal>(m_BulletRigidBody->getLinearDamping());
}
decimal ComponentRigidBody::getAngularDamping() const {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    return static_cast<decimal>(m_BulletRigidBody->getAngularDamping());
}
void ComponentRigidBody::collisionResponse(RigidCollisionCallbackEventData& data) const {
    if (m_CollisionFunctor) { //TODO: find out why this is needed and possibly remove this if check
        m_CollisionFunctor(data);
    }
}
MaskType ComponentRigidBody::getCollisionFlags() const {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    return static_cast<MaskType>(m_BulletRigidBody->getCollisionFlags());
}
void ComponentRigidBody::internal_setPosition(decimal x, decimal y, decimal z) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    btTransform tr;
    tr.setOrigin(btVector3{ (btScalar)x, (btScalar)y, (btScalar)z });
    tr.setRotation(m_BulletRigidBody->getOrientation());

    if (collisionShape && collisionShape->isStaticTriangleType()) {
        auto BTShape = collisionShape->getBtShape();
        m_BulletRigidBody->setCollisionShape(BTShape); //yes this is needed
        Engine::Physics::removeRigidBodyThreadSafe(getBtBody());
    }
    m_BulletMotionState.setWorldTransform(tr);
    m_BulletRigidBody->setMotionState(&m_BulletMotionState); //is this needed?
    m_BulletRigidBody->setWorldTransform(tr);
    m_BulletRigidBody->setCenterOfMassTransform(tr);
    if (collisionShape && collisionShape->isStaticTriangleType()) {
        Engine::Physics::addRigidBodyThreadSafe(getBtBody(), m_Group, m_Mask);
    }
}
void ComponentRigidBody::internal_calculate_mass() {
    if (m_Mass != 0.0f) {
        return;
    }
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        const auto& boundingBox = model->getBoundingBox();
        float volume            = boundingBox.x * boundingBox.y * boundingBox.z;
        float mass_             = (volume * 0.4f);
        setMass(mass_);
    }
}
void ComponentRigidBody::forcePhysicsSync() noexcept {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    auto transform = m_Owner.getComponent<ComponentTransform>();
    if (transform) {
        auto pos = transform->getPosition();
        auto rot = transform->getRotation();
        auto scl = transform->getScale();
        internal_setPosition(pos.x, pos.y, pos.z);
        internal_setRotation(rot.x, rot.y, rot.z, rot.w);
        internal_setScale(scl.x, scl.y, scl.z);
    }
}
void ComponentRigidBody::setGravity(decimal x, decimal y, decimal z) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setGravity(btVector3{ (btScalar)x, (btScalar)y, (btScalar)z });
}

void ComponentRigidBody::internal_set_matrix(glm_mat4 matrix) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    auto localScale = Engine::Math::removeMatrixScale<glm_mat4, glm_vec3>(matrix);
    btTransform tr;
    tr.setFromOpenGLMatrix(glm::value_ptr(matrix));
    m_BulletRigidBody->setWorldTransform(tr);
    m_BulletRigidBody->setCenterOfMassTransform(tr);
    m_BulletMotionState.setWorldTransform(tr);
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    if (collisionShape) {
        collisionShape->internal_setScale(localScale.x, localScale.y, localScale.z);
    }
}
void ComponentRigidBody::internal_setRotation(float x, float y, float z, float w) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    btQuaternion quat{ (btScalar)x, (btScalar)y, (btScalar)z, (btScalar)w };
    quat = quat.normalize();
    btTransform tr;
    tr.setOrigin(m_BulletRigidBody->getWorldTransform().getOrigin());
    tr.setRotation(quat);
    m_BulletRigidBody->setWorldTransform(tr);
    m_BulletRigidBody->setCenterOfMassTransform(tr);
    m_BulletMotionState.setWorldTransform(tr);
}
void ComponentRigidBody::internal_setScale(float x, float y, float z) {
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    if (collisionShape) {
        collisionShape->internal_setScale(x, y, z);
    }
}

glm_vec3 ComponentRigidBody::internal_getPosition() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    //btTransform tr;
    //m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    auto& tr = m_BulletRigidBody->getWorldTransform();
    return Engine::Math::toGLM(tr.getOrigin());
}
glm::quat ComponentRigidBody::internal_getRotation() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    //btTransform tr;
    //m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    auto& tr = m_BulletRigidBody->getWorldTransform();
    return Engine::Math::toGLM(tr.getRotation());
}
glm_vec3 ComponentRigidBody::getLinearVelocity() const {
    return Engine::Math::toGLM(m_BulletRigidBody->getLinearVelocity());
}
glm_vec3 ComponentRigidBody::getAngularVelocity() const {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    return Engine::Math::toGLM(m_BulletRigidBody->getAngularVelocity());
}
void ComponentRigidBody::setDamping(decimal linearFactor, decimal angularFactor) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setDamping((btScalar)linearFactor, (btScalar)angularFactor);
}
void ComponentRigidBody::setCollisionGroup(MaskType group) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Group = group;
    addPhysicsToWorld();
}
void ComponentRigidBody::setCollisionMask(MaskType mask) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Mask = mask;
    addPhysicsToWorld();
}
void ComponentRigidBody::addCollisionGroup(MaskType group) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Group |= group;
    addPhysicsToWorld();
}
void ComponentRigidBody::addCollisionMask(MaskType mask) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Mask |= mask;
    addPhysicsToWorld();
}
void ComponentRigidBody::setCollisionFlag(MaskType flag) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setCollisionFlags(flag);
}
void ComponentRigidBody::addCollisionFlag(MaskType flag) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setCollisionFlags(m_BulletRigidBody->getCollisionFlags() | flag);
}
void ComponentRigidBody::removeCollisionGroup(MaskType group) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Group &= ~group;
    addPhysicsToWorld();
}
void ComponentRigidBody::removeCollisionMask(MaskType mask) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Mask &= ~mask;
    addPhysicsToWorld();
}
void ComponentRigidBody::removeCollisionFlag(MaskType flag) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setCollisionFlags(m_BulletRigidBody->getCollisionFlags() & ~flag);
}
void ComponentRigidBody::setCollisionGroupAndMask(MaskType group, MaskType mask) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    removePhysicsFromWorld();
    m_Group = group;
    m_Mask  = mask;
    addPhysicsToWorld();
}

//TODO: reconsider how this works
void ComponentRigidBody::setDynamic(bool dynamic) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    if (dynamic) {
        m_BulletRigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
        m_BulletRigidBody->activate();
    }else{
        m_BulletRigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        ComponentRigidBody::clearAllForces();
        m_BulletRigidBody->activate();
    }
}
btVector3 ComponentRigidBody::internal_activate_and_get_vector(decimal x, decimal y, decimal z, bool local) noexcept {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->activate();
    btVector3 vec{ static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z) };
    Engine::Math::translate(*m_BulletRigidBody, vec, local);
    return vec;
}
void ComponentRigidBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setLinearVelocity(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentRigidBody::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setAngularVelocity(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentRigidBody::applyForce(decimal x, decimal y, decimal z, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->applyCentralForce(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentRigidBody::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    btVector3 v = internal_activate_and_get_vector(force.x, force.y, force.z, local);
    m_BulletRigidBody->applyForce(v, Engine::Math::toBT(origin));
}
void ComponentRigidBody::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->applyCentralImpulse(internal_activate_and_get_vector(x, y, z, local));
}
void ComponentRigidBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    btVector3 v = internal_activate_and_get_vector(impulse.x, impulse.y, impulse.z, local);
    m_BulletRigidBody->applyImpulse(v, Engine::Math::toBT(origin));
}
void ComponentRigidBody::applyTorque(decimal x, decimal y, decimal z, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    btVector3 v = internal_activate_and_get_vector(x, y, z, false); //yes, keep this false
    if (local) {
        v = m_BulletRigidBody->getInvInertiaTensorWorld().inverse() * (m_BulletRigidBody->getWorldTransform().getBasis() * v);
    }
    m_BulletRigidBody->applyTorque(v);
}
void ComponentRigidBody::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    btVector3 v = internal_activate_and_get_vector(x, y, z, false); //yes, keep this false
    if (local) {
        v = m_BulletRigidBody->getInvInertiaTensorWorld().inverse() * (m_BulletRigidBody->getWorldTransform().getBasis() * v);
    }
    m_BulletRigidBody->applyTorqueImpulse(v);
}
void ComponentRigidBody::clearLinearForces() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setActivationState(0);
    m_BulletRigidBody->activate();
    m_BulletRigidBody->setLinearVelocity(btVector3{ 0, 0, 0 });
}
void ComponentRigidBody::clearAngularForces() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_BulletRigidBody->setActivationState(0);
    m_BulletRigidBody->activate();
    m_BulletRigidBody->setAngularVelocity(btVector3{ 0, 0, 0 });
}
void ComponentRigidBody::clearAllForces() {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    clearLinearForces();
    clearAngularForces();
}
void ComponentRigidBody::setMass(float mass) {
    ASSERT(m_BulletRigidBody, __FUNCTION__ << "(): m_BulletRigidBody was null!");
    m_Mass = mass;
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    if (collisionShape) {
        collisionShape->calculateLocalInertia(mass);
        m_BulletRigidBody->setMassProps(mass, collisionShape->getInertia());
    }
    m_BulletRigidBody->updateInertiaTensor();
}
#pragma endregion

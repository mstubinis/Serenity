#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <serenity/physics/PhysicsModule.h>
#include <serenity/scene/Scene.h>
#include <serenity/ecs/ECS.h>
#include <serenity/resources/Engine_Resources.h>

namespace {
    btVector3 internal_activate_and_get_vector(btRigidBodyType* btRigidBody, decimal x, decimal y, decimal z, bool local) noexcept {
        assert(btRigidBody != nullptr);
        btRigidBody->activate();
        btVector3 vec{ btScalar(x), btScalar(y), btScalar(z) };
        Engine::Math::translate(*btRigidBody, vec, local);
        return vec;
    }
    glm::vec3 internal_getScale(Entity owner) noexcept {
        auto collisionShape = owner.getComponent<ComponentCollisionShape>();
        auto btColShape = collisionShape->getBtShape();
        if (btColShape) {
            return Engine::Math::toGLM(btColShape->getLocalScaling());
        }
        return glm::vec3{ 1.0f };
    }
}

ComponentRigidBody::ComponentRigidBody(Entity entity, CollisionFilter group, CollisionFilter mask, const std::string& name)
    : m_Owner { entity }
    , m_Group { group }
    , m_Mask  { mask }
{
    rebuildRigidBody(entity.scene().equals(Engine::Resources::getCurrentScene()));
#ifdef ENGINE_RIGID_BODY_ENHANCED
    setBtName(name);
#endif
}
ComponentRigidBody::ComponentRigidBody(Entity entity, const std::string& name)
    : ComponentRigidBody{ entity, CollisionFilter::DefaultFilter, CollisionFilter::AllFilter, name }
{}
ComponentRigidBody::~ComponentRigidBody() {
    cleanup();
}
void ComponentRigidBody::cleanup() {
    if (m_Owner) { //do not call from moved from destructors
        if (getBtBody()) {
            /*bool result = */removePhysicsFromWorld();
        }
    }
}
ComponentRigidBody::ComponentRigidBody(ComponentRigidBody&& other) noexcept {
    m_CollisionFunctor  = std::exchange(other.m_CollisionFunctor, [](RigidCollisionCallbackData&) {});
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
}
ComponentRigidBody& ComponentRigidBody::operator=(ComponentRigidBody&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_CollisionFunctor  = std::exchange(other.m_CollisionFunctor, [](RigidCollisionCallbackData&) {});
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
    }
    return *this;
}
ComponentRigidBody::Flags ComponentRigidBody::getFlags() const noexcept {
    assert(m_BulletRigidBody != nullptr);
    return m_BulletRigidBody->getFlags();
}
void ComponentRigidBody::setFlags(ComponentRigidBody::Flags flags) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setFlags(flags);
}
void ComponentRigidBody::addFlags(ComponentRigidBody::Flags flags) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setFlags(getFlags() | flags);
}
void ComponentRigidBody::removeFlags(ComponentRigidBody::Flags flags) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setFlags(getFlags() & ~flags);
}
void ComponentRigidBody::internal_update_misc() noexcept {
    if (m_BulletRigidBody) {
        m_BulletRigidBody->setMotionState(&m_BulletMotionState);
        auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
        if (collisionShape) {
            btCollisionShape* btShape = collisionShape->getBtShape();
            m_BulletRigidBody->setCollisionShape(btShape);
            setMass(m_Mass);
        }
        m_BulletRigidBody->updateInertiaTensor();
        m_BulletRigidBody->setUserPointer(this);
    }
}
bool ComponentRigidBody::rebuildRigidBody(bool addBodyToPhysicsWorld) {
    auto collisionShapeComponent = m_Owner.getComponent<ComponentCollisionShape>();
    auto transformComponent      = m_Owner.getComponent<ComponentTransform>();
    assert(transformComponent != nullptr);
    assert(collisionShapeComponent != nullptr);
    if (m_BulletRigidBody) {
        removePhysicsFromWorld();
    }
    btCollisionShape* btShape = collisionShapeComponent->getBtShape();
    btTransform t;
    if (transformComponent) {
        t.setOrigin(Engine::Math::toBT(transformComponent->getWorldPosition()));
        t.setRotation(Engine::Math::toBT(transformComponent->getWorldRotation()));
    } else {
        t.setIdentity();
    }
    m_BulletMotionState.setWorldTransform(t);
    btRigidBody::btRigidBodyConstructionInfo ci{ m_Mass, &m_BulletMotionState, btShape, collisionShapeComponent->getInertia() };
    ci.m_linearDamping            = btScalar(0.1);
    ci.m_angularDamping           = btScalar(0.4);
    ci.m_friction                 = btScalar(0.3);
    ci.m_linearSleepingThreshold  = btScalar(0.015);
    ci.m_angularSleepingThreshold = btScalar(0.015);

    m_BulletRigidBody.reset(new btRigidBodyType { ci });
    m_BulletRigidBody->setMassProps(m_Mass, collisionShapeComponent->getInertia());
    m_BulletRigidBody->updateInertiaTensor();
    m_BulletRigidBody->setUserPointer(this);
    internal_calculate_mass();
    internal_update_misc();
    if (addBodyToPhysicsWorld) {
        addPhysicsToWorld();
    }
    return (collisionShapeComponent != nullptr);
}
bool ComponentRigidBody::removePhysicsFromWorld() {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Physics::removeRigidBody(getBtBody());
}
bool ComponentRigidBody::addPhysicsToWorld() {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Physics::addRigidBody(getBtBody(), m_Group, m_Mask);
}
decimal ComponentRigidBody::getLinearDamping() const {
    assert(m_BulletRigidBody != nullptr);
    return decimal(m_BulletRigidBody->getLinearDamping());
}
decimal ComponentRigidBody::getAngularDamping() const {
    assert(m_BulletRigidBody != nullptr);
    return decimal(m_BulletRigidBody->getAngularDamping());
}
void ComponentRigidBody::collisionResponse(RigidCollisionCallbackData& data) const {
    //if (m_CollisionFunctor) { //TODO: find out why this is needed and possibly remove this if check
        m_CollisionFunctor(data);
    //}
}
MaskType ComponentRigidBody::getCollisionFlags() const {
    assert(m_BulletRigidBody != nullptr);
    return MaskType(m_BulletRigidBody->getCollisionFlags());
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
    assert(m_BulletRigidBody != nullptr);
    auto transform = m_Owner.getComponent<ComponentTransform>();
    if (transform) {
        internal_set_matrix(transform->getWorldMatrix());
    }
}
void ComponentRigidBody::setGravity(decimal x, decimal y, decimal z) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setGravity(btVector3{ (btScalar)x, (btScalar)y, (btScalar)z });
}
void ComponentRigidBody::internal_set_matrix(const glm_mat4& matrix) {
    assert(m_BulletRigidBody != nullptr);
    auto clone = matrix;
    auto localScale = Engine::Math::removeMatrixScale<glm_mat4, glm_vec3>(clone);
    btTransform tr;
    tr.setFromOpenGLMatrix(glm::value_ptr(clone));
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    /*
    if (collisionShape) {
        if (collisionShape->isStaticTriangleType()) {
            this->removePhysicsFromWorldImmediate();
        }
    }
    */
    m_BulletRigidBody->setWorldTransform(tr);
    m_BulletRigidBody->setCenterOfMassTransform(tr);
    m_BulletRigidBody->getMotionState()->setWorldTransform(tr);
    if (collisionShape) {
        const auto currentScale = internal_getScale(m_Owner);
        if (currentScale != glm::vec3{ localScale }) {
            collisionShape->internal_setScale(float(localScale.x), float(localScale.y), float(localScale.z));
        }
        /*
        if (collisionShape->isStaticTriangleType()) {
            this->addPhysicsToWorldImmediate();
        }
        */
    }
}
void ComponentRigidBody::internal_setScale(float x, float y, float z) {
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    if (collisionShape) {
        collisionShape->internal_setScale(x, y, z);
    }
}
btTransform ComponentRigidBody::internal_get_bt_transform() const {
    assert(m_BulletRigidBody != nullptr);
    return m_BulletRigidBody->getWorldTransform();
}
btTransform ComponentRigidBody::internal_get_bt_transform_motion_state() const {
    assert(m_BulletRigidBody != nullptr);
    btTransform tr;
    m_BulletRigidBody->getMotionState()->getWorldTransform(tr);
    return tr;
}
glm_vec3 ComponentRigidBody::getPosition() const {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Math::toGLM(internal_get_bt_transform().getOrigin());
}
glm::quat ComponentRigidBody::getRotation() const {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Math::toGLM(internal_get_bt_transform().getRotation());
}

glm_vec3 ComponentRigidBody::getPositionMotionState() const {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Math::toGLM(internal_get_bt_transform_motion_state().getOrigin());
}
glm::quat ComponentRigidBody::getRotationMotionState() const {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Math::toGLM(internal_get_bt_transform_motion_state().getRotation());
}
glm_mat4 ComponentRigidBody::getWorldMatrix() const {
    assert(m_BulletRigidBody != nullptr);
    glm_mat4 matrix;
    const auto& tr = internal_get_bt_transform();
    tr.getOpenGLMatrix(glm::value_ptr(matrix));
    return matrix;
}
glm_mat4 ComponentRigidBody::getWorldMatrixMotionState() const {
    assert(m_BulletRigidBody != nullptr);
    glm_mat4 matrix;
    const auto& tr = internal_get_bt_transform_motion_state();
    tr.getOpenGLMatrix(glm::value_ptr(matrix));
    return matrix;
}

glm_vec3 ComponentRigidBody::getLinearVelocity() const {
    return Engine::Math::toGLM(m_BulletRigidBody->getLinearVelocity());
}
glm_vec3 ComponentRigidBody::getAngularVelocity() const {
    assert(m_BulletRigidBody != nullptr);
    return Engine::Math::toGLM(m_BulletRigidBody->getAngularVelocity());
}
void ComponentRigidBody::setDamping(decimal linearFactor, decimal angularFactor) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setDamping(btScalar(linearFactor), btScalar(angularFactor));
}
void ComponentRigidBody::setCollisionGroup(CollisionFilter group) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Group = group;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}
void ComponentRigidBody::setCollisionMask(CollisionFilter mask) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Mask = mask;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}
void ComponentRigidBody::addCollisionGroup(CollisionFilter group) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Group |= group;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}
void ComponentRigidBody::addCollisionMask(CollisionFilter mask) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Mask |= mask;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}
void ComponentRigidBody::setCollisionFlag(CollisionFlag flag) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setCollisionFlags(flag);
}
void ComponentRigidBody::addCollisionFlag(CollisionFlag flag) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setCollisionFlags(m_BulletRigidBody->getCollisionFlags() | flag);
}
void ComponentRigidBody::removeCollisionGroup(CollisionFilter group) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Group &= ~group;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}
void ComponentRigidBody::removeCollisionMask(CollisionFilter mask) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Mask &= ~mask;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}
void ComponentRigidBody::removeCollisionFlag(CollisionFlag flag) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setCollisionFlags(m_BulletRigidBody->getCollisionFlags() & ~flag);
}
void ComponentRigidBody::setCollisionGroupAndMask(CollisionFilter group, CollisionFilter mask) noexcept {
    assert(m_BulletRigidBody != nullptr);
    m_Group = group;
    m_Mask  = mask;
    removePhysicsFromWorld();
    addPhysicsToWorld();
}

//TODO: reconsider how this works
bool ComponentRigidBody::isDynamic() const noexcept {
    return ((m_BulletRigidBody->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) == 0) && (getMass() > 0.0f);
}
void ComponentRigidBody::setDynamic(bool dynamic) {
    assert(m_BulletRigidBody != nullptr);
    if (dynamic) {
        m_BulletRigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
        m_BulletRigidBody->activate();
    } else {
        m_BulletRigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        ComponentRigidBody::clearAllForces();
        m_BulletRigidBody->activate();
    }
}
void ComponentRigidBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setLinearVelocity(internal_activate_and_get_vector(m_BulletRigidBody.get(), x, y, z, local));
}
void ComponentRigidBody::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setAngularVelocity(internal_activate_and_get_vector(m_BulletRigidBody.get(), x, y, z, local));
}
void ComponentRigidBody::applyForce(decimal x, decimal y, decimal z, bool local) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->applyCentralForce(internal_activate_and_get_vector(m_BulletRigidBody.get(), x, y, z, local));
}
void ComponentRigidBody::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    assert(m_BulletRigidBody != nullptr);
    btVector3 v = internal_activate_and_get_vector(m_BulletRigidBody.get(), force.x, force.y, force.z, local);
    m_BulletRigidBody->applyForce(v, Engine::Math::toBT(origin));
}
void ComponentRigidBody::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->applyCentralImpulse(internal_activate_and_get_vector(m_BulletRigidBody.get(), x, y, z, local));
}
void ComponentRigidBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    assert(m_BulletRigidBody != nullptr);
    btVector3 v = internal_activate_and_get_vector(m_BulletRigidBody.get(), impulse.x, impulse.y, impulse.z, local);
    m_BulletRigidBody->applyImpulse(v, Engine::Math::toBT(origin));
}
void ComponentRigidBody::applyTorque(decimal x, decimal y, decimal z, bool local) {
    assert(m_BulletRigidBody != nullptr);
    if (m_BulletRigidBody->getCollisionShape()->getShapeType() != CollisionType::EMPTY_SHAPE_PROXYTYPE) {
        btVector3 v = internal_activate_and_get_vector(m_BulletRigidBody.get(), x, y, z, false); //yes, keep this false
        if (local) {
            v = m_BulletRigidBody->getInvInertiaTensorWorld().inverse() * (m_BulletRigidBody->getWorldTransform().getBasis() * v);
        }
        m_BulletRigidBody->applyTorque(v);
    }
}
void ComponentRigidBody::applyTorqueX(decimal x, bool local) { applyTorque(x, decimal(0.0), decimal(0.0), local); }
void ComponentRigidBody::applyTorqueY(decimal y, bool local) { applyTorque(decimal(0.0), y, decimal(0.0), local); }
void ComponentRigidBody::applyTorqueZ(decimal z, bool local) { applyTorque(decimal(0.0), decimal(0.0), z, local); }

void ComponentRigidBody::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    assert(m_BulletRigidBody != nullptr);
    if (m_BulletRigidBody->getCollisionShape()->getShapeType() != CollisionType::EMPTY_SHAPE_PROXYTYPE) {
        btVector3 v = internal_activate_and_get_vector(m_BulletRigidBody.get(), x, y, z, false); //yes, keep this false
        if (local) {
            v = m_BulletRigidBody->getInvInertiaTensorWorld().inverse() * (m_BulletRigidBody->getWorldTransform().getBasis() * v);
        }
        m_BulletRigidBody->applyTorqueImpulse(v);
    }
}
void ComponentRigidBody::clearLinearForces() {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setActivationState(0);
    m_BulletRigidBody->activate();
    m_BulletRigidBody->setLinearVelocity(btVector3{ 0, 0, 0 });
}
void ComponentRigidBody::clearAngularForces() {
    assert(m_BulletRigidBody != nullptr);
    m_BulletRigidBody->setActivationState(0);
    m_BulletRigidBody->activate();
    m_BulletRigidBody->setAngularVelocity(btVector3{ 0, 0, 0 });
}
void ComponentRigidBody::clearAllForces() {
    assert(m_BulletRigidBody != nullptr);
    clearLinearForces();
    clearAngularForces();
}
void ComponentRigidBody::setMass(float mass) {
    assert(m_BulletRigidBody != nullptr);
    m_Mass = mass;
    auto collisionShape = m_Owner.getComponent<ComponentCollisionShape>();
    removePhysicsFromWorld();
    if (collisionShape) {
        collisionShape->calculateLocalInertia(mass);
        m_BulletRigidBody->setMassProps(mass, collisionShape->getInertia());
    }
    m_BulletRigidBody->updateInertiaTensor();
    addPhysicsToWorld();
}

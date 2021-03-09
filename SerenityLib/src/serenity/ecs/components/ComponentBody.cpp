#include <serenity/ecs/components/ComponentBody.h>
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

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region PhysicsData

ComponentBody::PhysicsData::~PhysicsData() {
    Physics::removeRigidBody(bullet_rigidBody.get());
}
ComponentBody::PhysicsData::PhysicsData(ComponentBody::PhysicsData&& other) noexcept 
    : mass               { std::move(other.mass) }
    , bullet_motionState { std::move(other.bullet_motionState) }
    , group              { std::move(other.group) }
    , mask               { std::move(other.mask) }
    , forcedOut          { std::move(other.forcedOut) }
    , collision          { std::move(other.collision) }
    , bullet_rigidBody   { std::move(other.bullet_rigidBody) }
{}
ComponentBody::PhysicsData& ComponentBody::PhysicsData::operator=(ComponentBody::PhysicsData&& other) noexcept {
    //move assignment
    ASSERT(&other != this, __FUNCTION__ << "(): trying to move itself!");
    mass               = std::move(other.mass);
    bullet_motionState = std::move(other.bullet_motionState);
    group              = std::move(other.group);
    mask               = std::move(other.mask);
    forcedOut          = std::move(other.forcedOut);

    collision          = std::move(other.collision);

    if (bullet_rigidBody) {
        Physics::removeRigidBody(bullet_rigidBody.get());
    }
    bullet_rigidBody   = std::move(other.bullet_rigidBody);
    if (bullet_rigidBody) {
        bullet_rigidBody->setCollisionShape(collision->getBtShape());
    }
    return *this;
}

#pragma endregion

#pragma region NormalData

ComponentBody::NormalData::NormalData(ComponentBody::NormalData&& other) noexcept 
    : position       { std::move(other.position) }
    , rotation       { std::move(other.rotation) }
    , scale          { std::move(other.scale) }
    , linearVelocity { std::move(other.linearVelocity) }
{}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(ComponentBody::NormalData&& other) noexcept {
    position       = std::move(other.position);
    rotation       = std::move(other.rotation);
    scale          = std::move(other.scale);
    linearVelocity = std::move(other.linearVelocity);
    return *this;
}


#pragma endregion

#pragma region Component

ComponentBody::ComponentBody(Entity entity) {
    m_Owner    = entity;
    m_Physics  = false;
    p          = nullptr;
    n          = std::make_unique<NormalData>();
    Math::recalculateForwardRightUp(n->rotation, m_Forward, m_Right, m_Up);
}
ComponentBody::ComponentBody(Entity entity, CollisionType collisionType) {
    m_Owner    = entity;
    m_Physics  = true;
    n          = nullptr;
    p          = std::make_unique<PhysicsData>();
    setCollision(collisionType, 1.0f);
    rebuildRigidBody(false);
}
ComponentBody::~ComponentBody() {
}
ComponentBody::ComponentBody(ComponentBody&& other) noexcept 
    : m_Physics          { std::move(other.m_Physics) }
    , m_Forward          { std::move(other.m_Forward) }
    , m_Right            { std::move(other.m_Right) }
    , m_Up               { std::move(other.m_Up) }
    , m_Owner            { std::move(other.m_Owner) }
    , m_CollisionFunctor { std::move(other.m_CollisionFunctor) }
    , m_UserPointer1     { std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2     { std::exchange(other.m_UserPointer2, nullptr) }
{
    n = std::move(other.n);
    p = std::move(other.p);

    m_UserPointer = std::exchange(other.m_UserPointer, nullptr);

    setInternalPhysicsUserPointer(this);
}
ComponentBody& ComponentBody::operator=(ComponentBody&& other) noexcept {
    //move assignment
    ASSERT(&other != this, __FUNCTION__ << "(): trying to move itself!");
    m_Physics          = std::move(other.m_Physics);
    m_Forward          = std::move(other.m_Forward);
    m_Right            = std::move(other.m_Right);
    m_Up               = std::move(other.m_Up);
    m_Owner            = std::move(other.m_Owner);
    m_CollisionFunctor = std::move(other.m_CollisionFunctor);
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);

    n.reset();
    p.reset();
    n = std::move(other.n);
    p = std::move(other.p);
    setInternalPhysicsUserPointer(this);
    return *this;
}
void ComponentBody::rebuildRigidBody(bool addBodyToPhysicsWorld, bool threadSafe) {
    if (m_Physics) {
        auto& inertia = p->collision->getBtInertia();
        auto* shape   = p->collision->getBtShape();
        if (shape) {
            if (p->bullet_rigidBody) {
                removePhysicsFromWorld(true, threadSafe);
            }
            p->bullet_rigidBody.reset(new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(p->mass, &p->bullet_motionState, shape, inertia)));
            p->bullet_rigidBody->setSleepingThresholds(0.015f, 0.015f);
            p->bullet_rigidBody->setFriction(0.3f);
            setDamping((decimal)(0.1), (decimal)(0.4));
            p->bullet_rigidBody->setMassProps((btScalar)(p->mass), inertia);
            p->bullet_rigidBody->updateInertiaTensor();
            setInternalPhysicsUserPointer(this);
            if (addBodyToPhysicsWorld) {
                addPhysicsToWorld(true, threadSafe);
            }
        }
    }
}
//kinda ugly
void ComponentBody::setInternalPhysicsUserPointer(void* userPtr) {
    if (m_Physics && p->bullet_rigidBody) {
        p->bullet_rigidBody->setUserPointer(userPtr);
        auto* shape = p->bullet_rigidBody->getCollisionShape();
        if (shape) {
            shape->setUserPointer(userPtr);
        }
    }
}
void ComponentBody::removePhysicsFromWorld(bool force, bool threadSafe) {
    if (force) {    
        p->forcedOut = true;
    }
    (threadSafe) ? Physics::removeRigidBodyThreadSafe(*this) : Physics::removeRigidBody(*this);
}
void ComponentBody::addPhysicsToWorld(bool force, bool threadSafe) {
    if (!force && p->forcedOut) {
        return;
    }
    (threadSafe) ? Physics::addRigidBodyThreadSafe(*this) : Physics::addRigidBody(*this);
    p->forcedOut = false;
}
decimal ComponentBody::getLinearDamping() const {
    return (p->bullet_rigidBody) ? (decimal)p->bullet_rigidBody->getLinearDamping() : decimal(0.0);
}
decimal ComponentBody::getAngularDamping() const {
    return (p->bullet_rigidBody) ? (decimal)p->bullet_rigidBody->getAngularDamping() : decimal(0.0);
}
void ComponentBody::collisionResponse(CollisionCallbackEventData& data) const {
    if (m_CollisionFunctor) { //TODO: find out why this is needed and possibly remove this if check
        m_CollisionFunctor(data);
    }
}
MaskType ComponentBody::getCollisionFlags() const {
    return (m_Physics) ? static_cast<MaskType>(p->bullet_rigidBody->getCollisionFlags()) : 0;
}
decimal ComponentBody::getDistance(Entity other) const {
    glm_vec3 other_position = other.getComponent<ComponentBody>()->getPosition();
    return glm::distance(getPosition(), other_position);
}
unsigned long long ComponentBody::getDistanceLL(Entity other) const {
    glm_vec3 other_position = other.getComponent<ComponentBody>()->getPosition();
    return (unsigned long long)glm::distance(getPosition(), other_position);
}
void ComponentBody::alignTo(decimal dirX, decimal dirY, decimal dirZ) {
    if (m_Physics) {
        //recheck this
        auto q = Math::alignTo(dirX, dirY, dirZ);
        ComponentBody::setRotation(q);
    }else{
        n->rotation = Math::alignTo(dirX, dirY, dirZ);
        Math::recalculateForwardRightUp(n->rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::alignTo(const glm_vec3& direction) {
    auto norm_dir = glm::normalize(direction);
    ComponentBody::alignTo(norm_dir.x, norm_dir.y, norm_dir.z);
}
void ComponentBody::internal_update_misc() noexcept {
    if (p->bullet_rigidBody) {
        p->bullet_rigidBody->setCollisionShape(p->collision->getBtShape());
        p->bullet_rigidBody->setMassProps((btScalar)p->mass, p->collision->getBtInertia());
        p->bullet_rigidBody->updateInertiaTensor();
    }
}
void ComponentBody::setCollision(CollisionType collisionType, float mass) {
    if (!p->collision) { //TODO: clean this up, its hacky and evil. its being used on the ComponentBody_EntityAddedToSceneFunction
        auto modelComponent = m_Owner.getComponent<ComponentModel>();
        if (modelComponent) {
            if (collisionType == CollisionType::Compound) {
                p->collision = std::make_unique<Collision>(*this, *modelComponent, mass);
            }else{
                p->collision = std::make_unique<Collision>(*this, collisionType, &modelComponent->getModel(), mass);
            }
        }else{
            p->collision = std::make_unique<Collision>(*this, collisionType, nullptr, mass);
        }
    }
    p->mass = mass;
    p->collision->setMass(p->mass);
    internal_update_misc();
    setInternalPhysicsUserPointer(this);
}
void ComponentBody::translate(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        btVector3 vec = internal_activate_and_get_vector(x, y, z, local);
        ComponentBody::setPosition(getPosition() + Math::btVectorToGLM(vec));
    }else{
        glm_vec3 offset(x, y, z);
        if (local) {
            offset = n->rotation * offset;
        }
		ComponentBody::setPosition(n->position + offset);
    }
}
void ComponentBody::rotate(decimal pitch, decimal yaw, decimal roll, bool local) {
    if (m_Physics) {
        auto& bt_rigidBody = *p->bullet_rigidBody;
        btQuaternion quat = bt_rigidBody.getWorldTransform().getRotation().normalize();
        glm_quat glmquat(quat.w(), quat.x(), quat.y(), quat.z());
        Math::rotate(glmquat, pitch, yaw, roll);
        quat = btQuaternion((btScalar)glmquat.x, (btScalar)glmquat.y, (btScalar)glmquat.z, (btScalar)glmquat.w);
        bt_rigidBody.getWorldTransform().setRotation(quat);
        Math::recalculateForwardRightUp(bt_rigidBody, m_Forward, m_Right, m_Up);
    }else{
        Math::rotate(n->rotation, pitch, yaw, roll);
        Math::recalculateForwardRightUp(n->rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::scale(decimal x, decimal y, decimal z) {
    if (m_Physics) {
        const auto newScale = btVector3((btScalar)x, (btScalar)y, (btScalar)z);
        auto collisionShape = p->collision->getBtShape();
        if (collisionShape) {
            collisionShape->setLocalScaling(collisionShape->getLocalScaling() + newScale);
        }
    }else{
        n->scale.x += x;
        n->scale.y += y;
        n->scale.z += z;
    }
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model) {
        ComponentModel_Functions::CalculateRadius(*model);
    }
}
void ComponentBody::setPosition(decimal x, decimal y, decimal z) {
    auto& ecs        = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system     = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
    auto& pcs        = system.ParentChildSystem;
    auto entityIndex = m_Owner.id() - 1U;
    if (m_Physics) {
        btTransform tr;
        tr.setOrigin(btVector3((btScalar)x, (btScalar)y, (btScalar)z));
        tr.setRotation(p->bullet_rigidBody->getOrientation());
        if (p->collision->getType() == CollisionType::TriangleShapeStatic) {
            removePhysicsFromWorld(false);
        }
        p->bullet_motionState.setWorldTransform(tr);
        p->bullet_rigidBody->setMotionState(&p->bullet_motionState); //is this needed?
        p->bullet_rigidBody->setWorldTransform(tr);
        p->bullet_rigidBody->setCenterOfMassTransform(tr);
        if (p->collision->getType() == CollisionType::TriangleShapeStatic) {
            addPhysicsToWorld(false);
        }
    }else{
        auto& position_     = n->position;

		position_.x         = x;
		position_.y         = y;
		position_.z         = z;
    }
    auto& localMatrix = pcs.LocalTransforms[entityIndex];
    localMatrix[3][0] = x;
    localMatrix[3][1] = y;
    localMatrix[3][2] = z;

    auto& worldMatrix = pcs.WorldTransforms[entityIndex];
    worldMatrix[3][0] = x;
    worldMatrix[3][1] = y;
    worldMatrix[3][2] = z;
}
void ComponentBody::setGravity(decimal x, decimal y, decimal z) {
    if (m_Physics) {
        p->bullet_rigidBody->setGravity(btVector3((btScalar)x, (btScalar)y, (btScalar)z));
    }
}
void ComponentBody::setRotation(decimal x, decimal y, decimal z, decimal w) {
    if (m_Physics) {
        btQuaternion quat((btScalar)x, (btScalar)y, (btScalar)z, (btScalar)w);
        quat              = quat.normalize();
        auto& rigidBody   = *p->bullet_rigidBody;
        btTransform tr; 
        tr.setOrigin(rigidBody.getWorldTransform().getOrigin());
        tr.setRotation(quat);
        rigidBody.setWorldTransform(tr);
        rigidBody.setCenterOfMassTransform(tr);
        p->bullet_motionState.setWorldTransform(tr);
        Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
        clearAngularForces();
    }else{
        glm_quat newRotation(w, x, y, z);
        newRotation = glm::normalize(newRotation);
        n->rotation = newRotation;
        Math::recalculateForwardRightUp(n->rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::setScale(decimal x, decimal y, decimal z) {
    if (m_Physics) {
        const auto newScale = btVector3((btScalar)x, (btScalar)y, (btScalar)z);
        Collision& collision_ = *p->collision;
        auto collisionShape = collision_.getBtShape();
        if (collisionShape) {
            collisionShape->setLocalScaling(newScale);
        }
    }else{
        n->scale.x = x;
        n->scale.y = y;
        n->scale.z = z;
    }
    auto models = m_Owner.getComponent<ComponentModel>();
    if (models) {
        priv::ComponentModel_Functions::CalculateRadius(*models);
    }
}
glm_vec3 ComponentBody::getLocalPosition() const { //theres prob a better way to do this
    if (m_Physics) {
        btTransform tr;
        p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        //auto& tr = p->bullet_rigidBody->getWorldTransform();
        return Math::btVectorToGLM(tr.getOrigin());
    }
    return n->position;
}
glm_vec3 ComponentBody::getPosition() const { //theres prob a better way to do this
    if (m_Physics) {
        btTransform tr;
        p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        //auto& tr = p->bullet_rigidBody->getWorldTransform();
        return Math::btVectorToGLM(tr.getOrigin());
    }
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
    auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return Math::getMatrixPosition(matrix);
}
glm::vec3 ComponentBody::getScreenCoordinates(bool clampToEdge) const {
	return Math::getScreenCoordinates(getPosition(), *m_Owner.scene()->getActiveCamera(), clampToEdge);
}
ScreenBoxCoordinates ComponentBody::getScreenBoxCoordinates(float minOffset) const {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = getPosition();
    auto radius             = 0.0001f;
    auto model              = m_Owner.getComponent<ComponentModel>();
    auto& camera            = *m_Owner.scene()->getActiveCamera();
    const auto center2DRes  = Math::getScreenCoordinates(worldPos, camera, false);
    const auto center2D     = glm::vec2(center2DRes.x, center2DRes.y);
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
    auto& cam                    = *Resources::getCurrentScene()->getActiveCamera();
    const auto camvectest        = cam.up();   
    const auto  testRes          = Math::getScreenCoordinates(worldPos + (camvectest * (decimal)radius), camera, false);
    const auto test              = glm::vec2(testRes.x, testRes.y);
    const auto radius2D          = glm::max(minOffset, glm::distance(test, center2D));
    const auto yPlus             = center2D.y + radius2D;
    const auto yNeg              = center2D.y - radius2D;
    const auto xPlus             = center2D.x + radius2D;
    const auto xNeg              = center2D.x - radius2D;
    ret.topLeft                  = glm::vec2(xNeg,  yPlus);
    ret.topRight                 = glm::vec2(xPlus, yPlus);
    ret.bottomLeft               = glm::vec2(xNeg,  yNeg);
    ret.bottomRight              = glm::vec2(xPlus, yNeg);
    ret.inBounds                 = center2DRes.z;
    return ret;
}
glm_vec3 ComponentBody::getScale() const {
    if (m_Physics) {
        Collision& collision_   = *p->collision;
        auto collisionShape     = collision_.getBtShape();
        if (collisionShape) {
            return Math::btVectorToGLM(collisionShape->getLocalScaling());
        }
        return glm_vec3((decimal)1.0);
    }
    return n->scale;
}
glm_quat ComponentBody::getRotation() const {
    if (m_Physics) {
        btTransform tr;
        p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        //auto& tr = p->bullet_rigidBody->getWorldTransform();
        auto quat = tr.getRotation();
        return Engine::Math::btToGLMQuat(quat);
    }
    return n->rotation;
}
glm_vec3 ComponentBody::getLinearVelocity() const  {
    if (m_Physics) {
        const btVector3& v = p->bullet_rigidBody->getLinearVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    return n->linearVelocity;
}
glm_vec3 ComponentBody::getAngularVelocity() const  {
    if (m_Physics) {
        const btVector3& v = p->bullet_rigidBody->getAngularVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    return glm_vec3((decimal)0.0);
}
glm_mat4 ComponentBody::modelMatrix() const { //theres prob a better way to do this
    if (m_Physics) {
#ifndef BT_USE_DOUBLE_PRECISION
        glm::mat4 modelMatrix_(1.0f);
#else
        glm_mat4 modelMatrix_((decimal)1.0);
#endif
        btTransform tr;
        p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        //auto& tr = p->bullet_rigidBody->getWorldTransform();
        btScalar* val_ptr = (btScalar*)glm::value_ptr(modelMatrix_);
        tr.getOpenGLMatrix(val_ptr);
        if (p->collision->getBtShape()) {
            auto scale   = getScale();
            modelMatrix_ = glm::scale(modelMatrix_, scale);
        }
        return modelMatrix_;
    }
    auto& ecs         = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system      = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
    auto& worldMatrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return worldMatrix;
}
void ComponentBody::setDamping(decimal linearFactor, decimal angularFactor) {
	p->bullet_rigidBody->setDamping((btScalar)linearFactor, (btScalar)angularFactor);
}
void ComponentBody::setCollisionGroup(MaskType group) {
    if (m_Physics) {
        removePhysicsFromWorld(true);
        p->group = group;
        addPhysicsToWorld(true);
    }
}
void ComponentBody::setCollisionMask(MaskType mask) {
    if (m_Physics) {
        removePhysicsFromWorld(true);
        p->mask = mask;
        addPhysicsToWorld(true);
    }
}
void ComponentBody::addCollisionGroup(MaskType group) {
    if (m_Physics) {
        removePhysicsFromWorld(true);
        p->group |= group;
        addPhysicsToWorld(true);
    }
}
void ComponentBody::addCollisionMask(MaskType mask) {
    if (m_Physics) {
        removePhysicsFromWorld(true);
        p->mask |= mask;
        addPhysicsToWorld(true);
    }
}
void ComponentBody::setCollisionFlag(MaskType flag) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        auto currFlags    = BtRigidBody.getCollisionFlags();
        if (currFlags != flag) {
            removePhysicsFromWorld(true);
            BtRigidBody.setCollisionFlags(flag);
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::addCollisionFlag(MaskType flag) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        auto currFlags    = BtRigidBody.getCollisionFlags();
        auto added_flags  = (currFlags | flag);
        if (currFlags != added_flags) {
            removePhysicsFromWorld(true);
            BtRigidBody.setCollisionFlags(added_flags);
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::removeCollisionGroup(MaskType group) {
    if (m_Physics) {
        removePhysicsFromWorld(true);
        p->group &= ~group;
        addPhysicsToWorld(true);
    }
}
void ComponentBody::removeCollisionMask(MaskType mask) {
    if (m_Physics) {
        removePhysicsFromWorld(true);
        p->mask &= ~mask;
        addPhysicsToWorld(true);
    }
}
void ComponentBody::removeCollisionFlag(MaskType flag) {
    if (m_Physics) {
        auto& BtRigidBody  = *p->bullet_rigidBody;
        auto currFlags     = BtRigidBody.getCollisionFlags();
        auto removed_flags = (currFlags & ~flag);
        if (currFlags != removed_flags) {
            removePhysicsFromWorld(true);
            BtRigidBody.setCollisionFlags(removed_flags);
            addPhysicsToWorld(true);
        }
    }
}

//TODO: reconsider how this works
void ComponentBody::setDynamic(bool dynamic) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        if (dynamic) {
            BtRigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            BtRigidBody.activate();
        }else{
            BtRigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            BtRigidBody.activate();
        }
    }
}
btVector3 ComponentBody::internal_activate_and_get_vector(decimal x, decimal y, decimal z, bool local) noexcept {
    p->bullet_rigidBody->activate();
    btVector3 vec((btScalar)x, (btScalar)y, (btScalar)z);
    Math::translate(*p->bullet_rigidBody, vec, local);
    return vec;
}
void ComponentBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(x, y, z, local);
        BtRigidBody.setLinearVelocity(v);
    }else{
        glm_vec3 offset(x, y, z);
        if (local) {
            offset = n->rotation * offset;
        }
        n->linearVelocity = offset;
    }
}
void ComponentBody::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
		auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(x, y, z, local);
        BtRigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::applyForce(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(x, y, z, local);
        BtRigidBody.applyCentralForce(v);
    }
}
void ComponentBody::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(force.x, force.y, force.z, local);
        BtRigidBody.applyForce(v, btVector3(static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z)));
    }
}
void ComponentBody::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(x, y, z, local);
        BtRigidBody.applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(impulse.x, impulse.y, impulse.z, local);
        BtRigidBody.applyImpulse(v, btVector3(static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z)));
    }
}
void ComponentBody::applyTorque(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(x, y, z, false); //yes, keep this false
        if (local) {
            v = BtRigidBody.getInvInertiaTensorWorld().inverse() * (BtRigidBody.getWorldTransform().getBasis() * v);
        }
        BtRigidBody.applyTorque(v);
    }
}
void ComponentBody::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        btVector3 v = internal_activate_and_get_vector(x, y, z, false); //yes, keep this false
        if (local) {
            v = BtRigidBody.getInvInertiaTensorWorld().inverse() * (BtRigidBody.getWorldTransform().getBasis() * v);
        }
        BtRigidBody.applyTorqueImpulse(v);
    }
}
void ComponentBody::clearLinearForces() {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        BtRigidBody.setActivationState(0);
        BtRigidBody.activate();
        BtRigidBody.setLinearVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAngularForces() {
    if (m_Physics) {
        auto& BtRigidBody = *p->bullet_rigidBody;
        BtRigidBody.setActivationState(0);
        BtRigidBody.activate();
        BtRigidBody.setAngularVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAllForces() {
    clearLinearForces();
    clearAngularForces();
}
void ComponentBody::setMass(float mass) {
    if (m_Physics) {
        auto& physicsData = *p;
        physicsData.mass = mass;
        Collision& collision = *physicsData.collision;
        if (collision.getBtShape()) {
            collision.setMass(physicsData.mass);
            if (physicsData.bullet_rigidBody) {
                physicsData.bullet_rigidBody->setMassProps((btScalar)(physicsData.mass), collision.getBtInertia());
            }
        }
    }
}
void ComponentBody::addChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
        auto& pcs    = system.ParentChildSystem;
        pcs.insert(m_Owner.id(), child.id());
    }
}
void ComponentBody::removeChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
        auto& system = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
        auto& pcs    = system.ParentChildSystem;
        pcs.remove(m_Owner.id(), child.id());
    }
}
void ComponentBody::removeAllChildren() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
    auto& pcs    = system.ParentChildSystem;

    for (size_t i = 0; i < pcs.Order.size(); ++i) {
        auto entityID = pcs.Order[i];
        if (entityID == m_Owner.id()) {
            size_t j = i + 1;
            while (j < pcs.Order.size() && pcs.Order[j] > 0) {
                if (pcs.Parents[pcs.Order[j] - 1U] == m_Owner.id()) {
                    pcs.remove(m_Owner.id(), pcs.Order[j]);
                }
                ++j;
            }
        }
    }
}
bool ComponentBody::hasParent() const {
    auto& ecs    = Engine::priv::PublicScene::GetECS(*m_Owner.scene());
    auto& system = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
    auto& pcs    = system.ParentChildSystem;
    return (pcs.Parents[m_Owner.id() - 1U] > 0);
}
void ComponentBody::internal_recalculateAllParentChildMatrices(SystemComponentBody& system) {
    auto& pcs = system.ParentChildSystem;
    for (size_t i = 0; i < pcs.Order.size(); ++i) {
        const uint32_t entityID = pcs.Order[i];
        if (entityID > 0) {
            const uint32_t entityIndex = entityID - 1U;
            const uint32_t parentID    = pcs.Parents[entityIndex];
            if (parentID == 0) {
                pcs.WorldTransforms[entityIndex] = pcs.LocalTransforms[entityIndex];
            }else{
                const uint32_t parentIndex = parentID - 1U;
                pcs.WorldTransforms[entityIndex] = pcs.WorldTransforms[parentIndex] * pcs.LocalTransforms[entityIndex];
            }
        }else{
            break;
        }
    }
}
void ComponentBody::recalculateAllParentChildMatrices(Scene& scene) {
    auto& ecs    = Engine::priv::PublicScene::GetECS(scene);
    auto& system = (SystemComponentBody&)ecs.getSystem<ComponentBody>();
    internal_recalculateAllParentChildMatrices(system);
}

#pragma endregion

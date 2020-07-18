#include "core/engine/utils/PrecompiledHeader.h"
#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/system/Engine.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/physics/Collision.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region CollisionCallbackParticipant
CollisionCallbackEventData::CollisionCallbackEventData(ComponentBody& a, ComponentBody& b, glm::vec3& c, glm::vec3& d, glm::vec3& e, glm::vec3& f, glm::vec3& g, glm::vec3& h, glm::vec3& i) :
ownerBody(a), otherBody(b), ownerHit(c), otherHit(d), normalOnB(e), ownerLocalHit(f), otherLocalHit(g), normalFromA(h), normalFromB(i) {

}
#pragma endregion

#pragma region PhysicsData

ComponentBody::PhysicsData::~PhysicsData() {
    Physics::removeRigidBody(bullet_rigidBody);
    SAFE_DELETE(collision);
    SAFE_DELETE(bullet_rigidBody);
}
ComponentBody::PhysicsData::PhysicsData(ComponentBody::PhysicsData&& other) noexcept {
    //move constructor
    mass               = std::move(other.mass);
    bullet_motionState = std::move(other.bullet_motionState);
    group              = std::move(other.group);
    mask               = std::move(other.mask);
    forcedOut          = std::move(other.forcedOut);
    collision          = std::exchange(other.collision, nullptr);
    bullet_rigidBody   = std::exchange(other.bullet_rigidBody, nullptr);  
}
ComponentBody::PhysicsData& ComponentBody::PhysicsData::operator=(ComponentBody::PhysicsData&& other) noexcept {
    //move assignment
    if(&other != this){
        mass               = std::move(other.mass);
        bullet_motionState = std::move(other.bullet_motionState);
        group              = std::move(other.group);
        mask               = std::move(other.mask);
        forcedOut          = std::move(other.forcedOut);

        SAFE_DELETE(collision);
        collision          = std::exchange(other.collision, nullptr);

        if (bullet_rigidBody) {
            Physics::removeRigidBody(bullet_rigidBody);
            SAFE_DELETE(bullet_rigidBody);
        }
        bullet_rigidBody   = std::exchange(other.bullet_rigidBody, nullptr);
        if (bullet_rigidBody) {
            bullet_rigidBody->setCollisionShape(collision->getBtShape());
        }
    }
    return *this;
}

#pragma endregion

#pragma region NormalData

ComponentBody::NormalData::NormalData(ComponentBody::NormalData&& other) noexcept {
    //move constructor
    position       = std::move(other.position);
    rotation       = std::move(other.rotation);
    scale          = std::move(other.scale);
    linearVelocity = std::move(other.linearVelocity);
}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(ComponentBody::NormalData&& other) noexcept {
    //move assignment
    if (&other != this) {
        position       = std::move(other.position);
        rotation       = std::move(other.rotation);
        scale          = std::move(other.scale);
        linearVelocity = std::move(other.linearVelocity);
    }
    return *this;
}


#pragma endregion

#pragma region Component

ComponentBody::ComponentBody(Entity entity) {
    m_Owner = entity;
    m_Physics                 = false;
    data.p                    = nullptr;
    data.n                    = NEW NormalData();
    auto& normalData          = *data.n;
    Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
}
ComponentBody::ComponentBody(Entity entity, CollisionType::Type collisionType) {
    m_Owner = entity;
    m_Physics               = true;
    data.n                  = nullptr;
    data.p                  = NEW PhysicsData();

    data.p->bullet_motionState = btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    setCollision(collisionType, 1.0f);
    rebuildRigidBody(false);
}
ComponentBody::~ComponentBody() {
    //destructor
    /*
    auto& ecs         = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system      = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
    auto& pcs         = system.ParentChildSystem;
    const auto thisID = m_Owner.id();
    */
    if (m_Physics) {
        SAFE_DELETE(data.p);
    }else{
        SAFE_DELETE(data.n);
    }
}
ComponentBody::ComponentBody(ComponentBody&& other) noexcept {
    //move constructor
    using std::swap;
    if (other.m_Physics) {
        data.p = std::exchange(other.data.p, nullptr);
    }else{
        data.n = std::exchange(other.data.n, nullptr);
    }
    m_Physics          = std::move(other.m_Physics);
    m_Forward          = std::move(other.m_Forward);
    m_Right            = std::move(other.m_Right);
    m_Up               = std::move(other.m_Up);
    m_Owner            = std::move(other.m_Owner);
    m_CollisionFunctor = std::move(other.m_CollisionFunctor);
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
    setInternalPhysicsUserPointer(this);
}
ComponentBody& ComponentBody::operator=(ComponentBody&& other) noexcept {
    //move assignment
    if (&other != this) {
        using std::swap;
        if (other.m_Physics) {
            SAFE_DELETE(data.p);
            data.p = std::exchange(other.data.p, nullptr);
        }else{
            SAFE_DELETE(data.n);
            data.n = std::exchange(other.data.n, nullptr);
        }
        m_Physics          = std::move(other.m_Physics);
        m_Forward          = std::move(other.m_Forward);
        m_Right            = std::move(other.m_Right);
        m_Up               = std::move(other.m_Up);
        m_Owner            = std::move(other.m_Owner);
        m_CollisionFunctor = std::move(other.m_CollisionFunctor);
        m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
        setInternalPhysicsUserPointer(this);
    }
    return *this;
}
void ComponentBody::setCollisionFunctor(function<void(CollisionCallbackEventData& data)> functor) {
    m_CollisionFunctor = functor;
}
void ComponentBody::onEvent(const Event& e) {

}

void ComponentBody::rebuildRigidBody(bool addBodyToPhysicsWorld, bool threadSafe) {
    if (m_Physics) {
        auto& p       = *data.p;
        auto& inertia = p.collision->getBtInertia();
        auto* shape   = p.collision->getBtShape();
        if (shape) {
            if (p.bullet_rigidBody) {
                removePhysicsFromWorld(true, threadSafe);
                SAFE_DELETE(p.bullet_rigidBody);
            }
            btRigidBody::btRigidBodyConstructionInfo CI(p.mass, &p.bullet_motionState, shape, inertia);
            p.bullet_rigidBody = new btRigidBody(CI);
            p.bullet_rigidBody->setSleepingThresholds(0.015f, 0.015f);
            p.bullet_rigidBody->setFriction(0.3f);
            ComponentBody::setDamping((decimal)(0.1), (decimal)(0.4));
            p.bullet_rigidBody->setMassProps((btScalar)(p.mass), inertia);
            p.bullet_rigidBody->updateInertiaTensor();
            setInternalPhysicsUserPointer(this);
            if (addBodyToPhysicsWorld) {
                addPhysicsToWorld(true, threadSafe);
            }
        }
    }
}
//kinda ugly
void ComponentBody::setInternalPhysicsUserPointer(void* userPtr) {
    if (m_Physics && data.p->bullet_rigidBody) {
        data.p->bullet_rigidBody->setUserPointer(userPtr);
        auto* shape = data.p->bullet_rigidBody->getCollisionShape();
        if (shape) {
            shape->setUserPointer(userPtr);
        }
    }
}
void ComponentBody::removePhysicsFromWorld(bool force, bool threadSafe) {
    if (force) {    
        data.p->forcedOut = true;
    }
    if (threadSafe) {
        Physics::removeRigidBodyThreadSafe(*this);
    }else{
        Physics::removeRigidBody(*this);
    }
}
void ComponentBody::addPhysicsToWorld(bool force, bool threadSafe) {
    if (!force && data.p->forcedOut) {
        return;
    }
    if (threadSafe) {
        Physics::addRigidBodyThreadSafe(*this);
    }else{
        Physics::addRigidBody(*this);
    }
    data.p->forcedOut = false;
}
decimal ComponentBody::getLinearDamping() const {
    return (data.p->bullet_rigidBody) ? (decimal)(data.p->bullet_rigidBody->getLinearDamping()) : decimal(0.0);
}
decimal ComponentBody::getAngularDamping() const {
    return (data.p->bullet_rigidBody) ? (decimal)(data.p->bullet_rigidBody->getAngularDamping()) : decimal(0.0);
}
void ComponentBody::collisionResponse(CollisionCallbackEventData& data) const {
    if (m_CollisionFunctor) { //TODO: find out why this is needed and possibly remove this if check
        m_CollisionFunctor(data);
    }
}
unsigned short ComponentBody::getCollisionGroup() const {
    return (m_Physics) ? data.p->group : 0;
}
unsigned short ComponentBody::getCollisionMask() const {
    return (m_Physics) ? data.p->mask : 0;
}
unsigned short ComponentBody::getCollisionFlags() const {
    return (m_Physics) ? data.p->bullet_rigidBody->getCollisionFlags() : 0;
}

decimal ComponentBody::getDistance(Entity other) const {
    glm_vec3 other_position = other.getComponent<ComponentBody>()->getPosition();
    return glm::distance(getPosition(), other_position);
}
unsigned long long ComponentBody::getDistanceLL(Entity other) const {
    glm_vec3 other_position = other.getComponent<ComponentBody>()->getPosition();
    return (unsigned long long)glm::distance(getPosition(), other_position);
}
void ComponentBody::alignTo(const glm_vec3& direction) {
    if (m_Physics) {
        //recheck this
        glm_quat q(1.0, 0.0, 0.0, 0.0);
        Math::alignTo(q, glm::normalize(direction));
        ComponentBody::setRotation(q);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(normalData.rotation, glm::normalize(direction));
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
Collision* ComponentBody::getCollision() const {
    if (m_Physics) {
        return data.p->collision;
    }
    return nullptr;
}
void ComponentBody::setCollision(CollisionType::Type collisionType, float mass) {
    if (!data.p->collision) { //TODO: clean this up, its hacky and evil. its being used on the ComponentBody_EntityAddedToSceneFunction
        auto* modelComponent = m_Owner.getComponent<ComponentModel>();
        if (modelComponent) {
            if (collisionType == CollisionType::Compound) {
                data.p->collision = NEW Collision(*this, *modelComponent, mass);
            }else{
                data.p->collision = NEW Collision(*this, collisionType, &modelComponent->getModel(), mass);
            }
        }else{
            data.p->collision = NEW Collision(*this, collisionType, nullptr, mass);
        }
    }
    data.p->mass = mass;
    data.p->collision->setMass(data.p->mass);
    if (data.p->bullet_rigidBody) {
        data.p->bullet_rigidBody->setCollisionShape(data.p->collision->getBtShape());
        data.p->bullet_rigidBody->setMassProps((btScalar)data.p->mass, data.p->collision->getBtInertia());
        data.p->bullet_rigidBody->updateInertiaTensor();
    }
    setInternalPhysicsUserPointer(this);
}
//double check this...
void ComponentBody::setCollision(Collision* collision) {
    if (data.p->collision) {
        removePhysicsFromWorld(false, false);
        SAFE_DELETE(data.p->collision);
    }
    data.p->collision = collision;
    if (data.p->bullet_rigidBody) {
        data.p->bullet_rigidBody->setCollisionShape(data.p->collision->getBtShape());
        data.p->bullet_rigidBody->setMassProps((btScalar)data.p->mass, data.p->collision->getBtInertia());
        data.p->bullet_rigidBody->updateInertiaTensor();
        addPhysicsToWorld(false, false);
    }
    setInternalPhysicsUserPointer(this);
}

void ComponentBody::translate(const glm_vec3& translation, bool local) {
	ComponentBody::translate(translation.x, translation.y, translation.z, local);
}
void ComponentBody::translate(decimal translation, bool local) {
	ComponentBody::translate(translation, translation, translation, local);
}
void ComponentBody::translate(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        data.p->bullet_rigidBody->activate();
        btVector3 vec((btScalar)x, (btScalar)y, (btScalar)z);
        Math::translate(*data.p->bullet_rigidBody, vec, local);
        ComponentBody::setPosition(getPosition() + Math::btVectorToGLM(vec));
    }else{
        auto& normalData = *data.n;
        glm_vec3 offset(x, y, z);
        if (local) {
            offset = normalData.rotation * offset;
        }
		ComponentBody::setPosition(normalData.position + offset);
    }
}
void ComponentBody::rotate(const glm_vec3& rotation, bool local) {
	ComponentBody::rotate(rotation.x, rotation.y, rotation.z, local);
}
void ComponentBody::rotate(decimal pitch, decimal yaw, decimal roll, bool local) {
    if (m_Physics) {
        auto& bt_rigidBody = *data.p->bullet_rigidBody;
        btQuaternion quat = bt_rigidBody.getWorldTransform().getRotation().normalize();
        glm_quat glmquat(quat.w(), quat.x(), quat.y(), quat.z());
        Math::rotate(glmquat, pitch, yaw, roll);
        quat = btQuaternion((btScalar)glmquat.x, (btScalar)glmquat.y, (btScalar)glmquat.z, (btScalar)glmquat.w);
        bt_rigidBody.getWorldTransform().setRotation(quat);
        Math::recalculateForwardRightUp(bt_rigidBody, m_Forward, m_Right, m_Up);
    }else{
        auto& normalData = *data.n;
        Math::rotate(normalData.rotation, pitch, yaw, roll);
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::scale(const glm_vec3& scaleAmount) { 
	ComponentBody::scale(scaleAmount.x, scaleAmount.y, scaleAmount.z);
}
void ComponentBody::scale(decimal scaleAmount) {
	ComponentBody::scale(scaleAmount, scaleAmount, scaleAmount);
}
void ComponentBody::scale(decimal x, decimal y, decimal z) {
    if (m_Physics) {
        const auto newScale = btVector3((btScalar)x, (btScalar)y, (btScalar)z);
        Collision& collision_ = *data.p->collision;
        auto collisionShape = collision_.getBtShape();
        if (collisionShape) {
            collisionShape->setLocalScaling(collisionShape->getLocalScaling() + newScale);
        }
    }else{
        auto& scl = data.n->scale;
        scl.x += x;
        scl.y += y;
        scl.z += z;
    }
    auto* models = m_Owner.getComponent<ComponentModel>();
    if (models) {
        ComponentModel_Functions::CalculateRadius(*models);
    }
}
void ComponentBody::setPosition(const glm_vec3& newPosition) {
	ComponentBody::setPosition(newPosition.x, newPosition.y, newPosition.z);
}
void ComponentBody::setPosition(decimal newPosition) {
	ComponentBody::setPosition(newPosition, newPosition, newPosition);
}
void ComponentBody::setPosition(decimal x, decimal y, decimal z) {
    auto& ecs        = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system     = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
    auto& pcs        = system.ParentChildSystem;
    auto entityIndex = m_Owner.id() - 1U;
    if (m_Physics) {
        btTransform tr;
        tr.setOrigin(btVector3((btScalar)x, (btScalar)y, (btScalar)z));
        tr.setRotation(data.p->bullet_rigidBody->getOrientation());
        if (data.p->collision->getType() == CollisionType::TriangleShapeStatic) {
            removePhysicsFromWorld(false);
        }
        data.p->bullet_motionState.setWorldTransform(tr);
        data.p->bullet_rigidBody->setMotionState(&data.p->bullet_motionState); //is this needed?
        data.p->bullet_rigidBody->setWorldTransform(tr);
        data.p->bullet_rigidBody->setCenterOfMassTransform(tr);
        if (data.p->collision->getType() == CollisionType::TriangleShapeStatic) {
            addPhysicsToWorld(false);
        }
    }else{
        auto& normalData    = *data.n;
        auto& position_     = normalData.position;

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
        auto& physicsData = *data.p;
        physicsData.bullet_rigidBody->setGravity(btVector3((btScalar)x, (btScalar)y, (btScalar)z));
    }
}
void ComponentBody::setRotation(const glm_quat& newRotation) {
	ComponentBody::setRotation(newRotation.x, newRotation.y, newRotation.z, newRotation.w);
}
void ComponentBody::setRotation(decimal x, decimal y, decimal z, decimal w) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        btQuaternion quat((btScalar)x, (btScalar)y, (btScalar)z, (btScalar)w);
        quat              = quat.normalize();
        auto& rigidBody   = *physicsData.bullet_rigidBody;
        btTransform tr; 
        tr.setOrigin(rigidBody.getWorldTransform().getOrigin());
        tr.setRotation(quat);
        rigidBody.setWorldTransform(tr);
        rigidBody.setCenterOfMassTransform(tr);
        physicsData.bullet_motionState.setWorldTransform(tr);
        Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
        clearAngularForces();
    }else{
        auto& normalData = *data.n;
        glm_quat newRotation(w, x, y, z);
        newRotation = glm::normalize(newRotation);
        normalData.rotation = newRotation;
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::setScale(const glm_vec3& newScale) {
	ComponentBody::setScale(newScale.x, newScale.y, newScale.z);
}
void ComponentBody::setScale(decimal newScale) {
	ComponentBody::setScale(newScale, newScale, newScale);
}
void ComponentBody::setScale(decimal x, decimal y, decimal z) {
    if (m_Physics) {
        const auto newScale = btVector3((btScalar)x, (btScalar)y, (btScalar)z);
        Collision& collision_ = *data.p->collision;
        auto collisionShape = collision_.getBtShape();
        if (collisionShape) {
            collisionShape->setLocalScaling(newScale);
        }
    }else{
        auto& scl = data.n->scale;
        scl.x = x;
        scl.y = y;
        scl.z = z;
    }
    auto* models = m_Owner.getComponent<ComponentModel>();
    if (models) {
        priv::ComponentModel_Functions::CalculateRadius(*models);
    }
}
glm_vec3 ComponentBody::getLocalPosition() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        return Math::btVectorToGLM(tr.getOrigin());
    }
    return data.n->position;
}
glm_vec3 ComponentBody::getPosition() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        return Math::btVectorToGLM(tr.getOrigin());
    }
    auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
    auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return Math::getMatrixPosition(matrix);
}
glm::vec3 ComponentBody::getPositionRender() const { //theres prob a better way to do this
    if (m_Physics) {
        auto tr = data.p->bullet_rigidBody->getWorldTransform();
        return Math::btVectorToGLM(tr.getOrigin());
    }
    auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
    auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return Math::getMatrixPosition(matrix);
}
glm::vec3 ComponentBody::getScreenCoordinates(bool clampToEdge) const {
	return Math::getScreenCoordinates(getPosition(), *m_Owner.scene().getActiveCamera(), clampToEdge);
}
ScreenBoxCoordinates ComponentBody::getScreenBoxCoordinates(float minOffset) const {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = getPosition();
    auto radius             = 0.0001f;
    auto* model             = m_Owner.getComponent<ComponentModel>();
    auto& camera            = *m_Owner.scene().getActiveCamera();
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
        const auto& physicsData = *data.p;
        Collision& collision_   = *physicsData.collision;
        auto collisionShape     = collision_.getBtShape();
        if (collisionShape) {
            return Math::btVectorToGLM(collisionShape->getLocalScaling());
        }
        return glm_vec3((decimal)1.0);
    }
    return data.n->scale;
}
glm_quat ComponentBody::getRotation() const {
    if (m_Physics) {
        btTransform tr;
        data.p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        auto quat = tr.getRotation();
        return Engine::Math::btToGLMQuat(quat);
    }
    return data.n->rotation;
}
glm_vec3 ComponentBody::getLinearVelocity() const  {
    if (m_Physics) {
        const btVector3& v = data.p->bullet_rigidBody->getLinearVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    auto& normalData = *data.n;
    return normalData.linearVelocity;
}
glm_vec3 ComponentBody::getAngularVelocity() const  {
    if (m_Physics) {
        const btVector3& v = data.p->bullet_rigidBody->getAngularVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    return glm_vec3((decimal)0.0);
}
float ComponentBody::mass() const {
	return (m_Physics) ? data.p->mass : 0.0f;
}
glm_mat4 ComponentBody::modelMatrix() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
#ifndef BT_USE_DOUBLE_PRECISION
        glm::mat4 modelMatrix_(1.0f);
#else
        glm_mat4 modelMatrix_((decimal)1.0);
#endif
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        btScalar* val_ptr = (btScalar*)glm::value_ptr(modelMatrix_);
        tr.getOpenGLMatrix(val_ptr);
        auto& collision_  = *physicsData.collision;
        if (collision_.getBtShape()) {
            auto scale   = getScale();
            modelMatrix_ = glm::scale(modelMatrix_, scale);
        }
        return modelMatrix_;
    }
    auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
    auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return matrix;
}
glm::mat4 ComponentBody::modelMatrixRendering() const {
    return (glm::mat4)modelMatrix();
}
btRigidBody& ComponentBody::getBtBody() const {
	return *data.p->bullet_rigidBody;
}
void ComponentBody::setDamping(decimal linearFactor, decimal angularFactor) {
	data.p->bullet_rigidBody->setDamping(btScalar(linearFactor), btScalar(angularFactor));
}
void ComponentBody::setCollisionGroup(short group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != group) {
            removePhysicsFromWorld(true);
            phyData.group = group;
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::setCollisionMask(short mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != mask) {
            removePhysicsFromWorld(true);
            phyData.mask = mask;
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::setCollisionGroup(CollisionFilter::Filter group) {
    ComponentBody::setCollisionGroup((short)group);
}
void ComponentBody::setCollisionMask(CollisionFilter::Filter mask) {
    ComponentBody::setCollisionMask((short)mask);
}
void ComponentBody::addCollisionGroup(short group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto added_flags = (phyData.group | group);
        if (phyData.group != added_flags) {
            removePhysicsFromWorld(true);
            phyData.group = added_flags;
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::addCollisionMask(short mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto added_flags = (phyData.mask | mask);
        if (phyData.mask != added_flags) {
            removePhysicsFromWorld(true);
            phyData.mask = added_flags;
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::addCollisionGroup(CollisionFilter::Filter group) {
    ComponentBody::addCollisionGroup((short)group);
}
void ComponentBody::addCollisionMask(CollisionFilter::Filter mask) {
    ComponentBody::addCollisionMask((short)mask);
}
void ComponentBody::setCollisionFlag(short flag) {
    if (m_Physics) {
        auto& phyData   = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        auto currFlags  = rigidBody.getCollisionFlags();
        if (currFlags != flag) {
            removePhysicsFromWorld(true);
            rigidBody.setCollisionFlags(flag);
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::setCollisionFlag(CollisionFlag::Flag flag) {
    ComponentBody::setCollisionFlag((short)flag);
}
void ComponentBody::addCollisionFlag(short flag) {
    if (m_Physics) {
        auto& phyData    = *data.p;
        auto& rigidBody  = *phyData.bullet_rigidBody;
        auto currFlags   = rigidBody.getCollisionFlags();
        auto added_flags = (currFlags | flag);
        if (currFlags != added_flags) {
            removePhysicsFromWorld(true);
            rigidBody.setCollisionFlags(added_flags);
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::addCollisionFlag(CollisionFlag::Flag flag) {
    ComponentBody::addCollisionFlag((short)flag);
}
void ComponentBody::removeCollisionGroup(short group) {
    if (m_Physics) {
        auto& phyData      = *data.p;
        auto removed_flags = (phyData.group & ~group);
        if (phyData.group != removed_flags) {
            removePhysicsFromWorld(true);
            phyData.group = removed_flags;
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::removeCollisionMask(short mask) {
    if (m_Physics) {
        auto& phyData      = *data.p;
        auto removed_flags = (phyData.mask & ~mask);
        if (phyData.mask != removed_flags) {
            removePhysicsFromWorld(true);
            phyData.mask = removed_flags;
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::removeCollisionFlag(short flag) {
    if (m_Physics) {
        auto& phyData      = *data.p;
        auto& rigidBody    = *phyData.bullet_rigidBody;
        auto currFlags     = rigidBody.getCollisionFlags();
        auto removed_flags = (currFlags & ~flag);
        if (currFlags != removed_flags) {
            removePhysicsFromWorld(true);
            rigidBody.setCollisionFlags(removed_flags);
            addPhysicsToWorld(true);
        }
    }
}
void ComponentBody::removeCollisionGroup(CollisionFilter::Filter group) {
    ComponentBody::removeCollisionGroup((short)group);
}
void ComponentBody::removeCollisionMask(CollisionFilter::Filter mask) {
    ComponentBody::removeCollisionMask((short)mask);
}
void ComponentBody::removeCollisionFlag(CollisionFlag::Flag flag) {
    ComponentBody::removeCollisionFlag((short)flag);
}


//TODO: reconsider how this works
void ComponentBody::setDynamic(bool dynamic) {
    if (m_Physics) {
        auto& p = *data.p;
        auto& rigidBody = *p.bullet_rigidBody;
        if (dynamic) {
            //ComponentBody::removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            //ComponentBody::addPhysicsToWorld(false);
            rigidBody.activate();
        }else{
            //ComponentBody::removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            //ComponentBody::addPhysicsToWorld(false);
            rigidBody.activate();
        }
    }
}
void ComponentBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v((btScalar)x, (btScalar)y, (btScalar)z);
        Math::translate(rigidBody, v, local);
        rigidBody.setLinearVelocity(v);
    }else{
        auto& normalData = *data.n;
        glm_vec3 offset(x, y, z);
        if (local) {
            offset = normalData.rotation * offset;
        }
        normalData.linearVelocity = offset;
    }
}
void ComponentBody::setLinearVelocity(const glm_vec3& velocity, bool local) {
	ComponentBody::setLinearVelocity(velocity.x, velocity.y, velocity.z, local);
}
void ComponentBody::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
		auto& rigidBody = *data.p->bullet_rigidBody;
		rigidBody.activate();
		btVector3 v((btScalar)x, (btScalar)y, (btScalar)z);
		Math::translate(rigidBody, v, local);
		rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setAngularVelocity(const glm_vec3& velocity, bool local) {
	ComponentBody::setAngularVelocity(velocity.x, velocity.y, velocity.z, local);
}
void ComponentBody::applyForce(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyCentralForce(v);
    }
}
void ComponentBody::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(force.x), static_cast<btScalar>(force.y), static_cast<btScalar>(force.z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyForce(v, btVector3(static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z)));
    }
}
void ComponentBody::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(impulse.x), static_cast<btScalar>(impulse.y), static_cast<btScalar>(impulse.z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyImpulse(v, btVector3(static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z)));
    }
}
void ComponentBody::applyTorque(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 t(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
        if (local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorque(t);
    }
}
void ComponentBody::applyTorque(const glm_vec3& torque, bool local) {
	ComponentBody::applyTorque(torque.x, torque.y, torque.z, local);
}
void ComponentBody::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 t(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
        if (local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorqueImpulse(t);
    }
}
void ComponentBody::applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local) {
	ComponentBody::applyTorqueImpulse(torqueImpulse.x, torqueImpulse.y, torqueImpulse.z, local);
}
void ComponentBody::clearLinearForces() {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setLinearVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAngularForces() {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setAngularVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAllForces() {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        btVector3 v(0, 0, 0);
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setLinearVelocity(v);
        rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setMass(float mass) {
    if (m_Physics) {
        auto& physicsData = *data.p;
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
        auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
        auto& system = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
        auto& pcs    = system.ParentChildSystem;
        pcs.insert(m_Owner.id(), child.id());
    }
}
void ComponentBody::addChild(const ComponentBody& child) const {
    ComponentBody::addChild(child.m_Owner);
}
void ComponentBody::removeChild(Entity child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
        auto& system = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
        auto& pcs    = system.ParentChildSystem;
        pcs.remove(m_Owner.id(), child.id());
    }
}
void ComponentBody::removeChild(const ComponentBody& child) const {
    ComponentBody::removeChild(child.m_Owner);
}
bool ComponentBody::hasParent() const {
    auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = (Engine::priv::ComponentBody_System&)ecs.getSystem<ComponentBody>();
    auto& pcs    = system.ParentChildSystem;
    return (pcs.Parents[m_Owner.id() - 1U] > 0);
}
void ComponentBody::internal_recalculateAllParentChildMatrices(ComponentBody_System& system) {
    auto& pcs = system.ParentChildSystem;
    for (size_t i = 0; i < pcs.Order.size(); ++i) {
        unsigned int entityID = pcs.Order[i];
        if (entityID > 0) {
            unsigned int entityIndex = entityID - 1U;
            unsigned int parentID = pcs.Parents[entityIndex];
            if (parentID == 0) {
                pcs.WorldTransforms[entityIndex] = pcs.LocalTransforms[entityIndex];
            }else{
                unsigned int parentIndex = parentID - 1U;
                pcs.WorldTransforms[entityIndex] = pcs.WorldTransforms[parentIndex] * pcs.LocalTransforms[entityIndex];
            }
        }else{
            break;
        }
    }
}
void ComponentBody::recalculateAllParentChildMatrices(Scene& scene) {
    auto& ecs    = Engine::priv::InternalScenePublicInterface::GetECS(scene);
    auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
    internal_recalculateAllParentChildMatrices(system);
}


#pragma endregion

#pragma region SystemCI

constexpr glm_mat4 IDENTITY_MATRIX = glm_mat4(1.0);

struct priv::ComponentBody_UpdateFunction final { void operator()(void* systemPtr, void* componentPool, const float dt, Scene& scene) const {
    auto& system                = *static_cast<Engine::priv::ComponentBody_System*>(systemPtr);
    auto& pool                  = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(componentPool);
    auto& components            = pool.data();

    auto lamda_update_component = [dt, &system](ComponentBody& b, size_t i, size_t k) {
        const auto entityIndex  = b.m_Owner.id() - 1U;
        auto& localMatrix       = system.ParentChildSystem.LocalTransforms[entityIndex];
        auto& worldMatrix       = system.ParentChildSystem.WorldTransforms[entityIndex];
        if (b.m_Physics) {
            auto& rigidBody     = *b.data.p->bullet_rigidBody;
            Engine::Math::recalculateForwardRightUp(rigidBody, b.m_Forward, b.m_Right, b.m_Up);

            localMatrix         = b.modelMatrix();
            worldMatrix         = localMatrix;
        }else{
            auto& n             = *b.data.n;
            n.position         += (n.linearVelocity * static_cast<decimal>(dt));
            //n.modelMatrix     = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);

            localMatrix         = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);
            worldMatrix         = localMatrix;
        }
    };

    if (components.size() < 200) {
        for (size_t i = 0; i < components.size(); ++i) {
            lamda_update_component(components[i], i, 0);
        }
    }else{
        Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
    }
    
    ComponentBody::internal_recalculateAllParentChildMatrices(system);

#if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
    for (auto& componentBody : components) {
        Entity entity      = componentBody.getOwner();
        auto bodyRenderPos = componentBody.getPosition();
        ComponentModel* model        = entity.getComponent<ComponentModel>();
        if (model) {
            auto world_pos = glm::vec3(componentBody.getPosition());
            auto world_rot = glm::quat(componentBody.getRotation());
            auto world_scl = glm::vec3(componentBody.getScale());
            for (size_t i = 0; i < model->getNumModels(); ++i) {
                auto& modelInstance = (*model)[i];

                auto rotation = world_rot * modelInstance.orientation();
                auto fwd      = glm::normalize(Math::getForward(rotation)) * 0.3f;
                auto right    = glm::normalize(Math::getRight(rotation)) * 0.3f;
                auto up       = glm::normalize(Math::getUp(rotation)) * 0.3f;

                auto& physics = Engine::priv::Core::m_Engine->m_PhysicsManager;
                physics.debug_draw_line(world_pos, (world_pos+fwd) /* * glm::length(world_scl) */, 1, 0, 0, 1);
                physics.debug_draw_line(world_pos, (world_pos+right) /* * glm::length(world_scl) */, 0, 1, 0, 1);
                physics.debug_draw_line(world_pos, (world_pos+up) /* * glm::length(world_scl) */, 0, 0, 1, 1);
            }
        }
        const auto screenPos = Math::getScreenCoordinates(bodyRenderPos, *scene.getActiveCamera(), false);
        if (screenPos.z > 0) {
            const string text = "ID: " + to_string(entity.id());
            Font::renderTextStatic(text, glm::vec2(screenPos.x, screenPos.y), glm::vec4(1.0f), 0.0f, glm::vec2(0.5f), 0.1f, TextAlignment::Left);
        }
    }
#endif

}};
struct priv::ComponentBody_ComponentAddedToEntityFunction final {void operator()(void* systemPtr, void* component, Entity entity) const {
    auto& system  = *static_cast<Engine::priv::ComponentBody_System*>(systemPtr);
    auto& pcs     = system.ParentChildSystem;
    const auto id = entity.id();

    if (pcs.Parents.capacity() < id) {
        pcs.reserve(id + 50U);
    }
    if (pcs.Parents.size() < id) {
        pcs.resize(id);
    }
}};
struct priv::ComponentBody_ComponentRemovedFromEntityFunction final { void operator()(void* systemPtr, Entity entity) const {
    auto& system         = *static_cast<Engine::priv::ComponentBody_System*>(systemPtr);
    const auto id        = entity.id();
    auto& pcs            = system.ParentChildSystem;
    const auto thisIndex = id - 1U;

    if (pcs.Parents[thisIndex] > 0) {
        pcs.remove(pcs.Parents[thisIndex], id);
    }
}};
struct priv::ComponentBody_EntityAddedToSceneFunction final {void operator()(void* systemPtr, void* componentPool, Entity entity, Scene& scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(componentPool);
    auto* component_ptr = pool.getComponent(entity);
    if (component_ptr) {
        auto& component = *component_ptr;
        if (component.m_Physics) {
            auto& physicsData = *component.data.p;
            component.setCollision(static_cast<CollisionType::Type>(physicsData.collision->getType()), physicsData.mass);
            auto currentScene = Resources::getCurrentScene();
            if (currentScene && currentScene == &scene) {
                component.addPhysicsToWorld(true);
            }
        }
    }
}};
struct priv::ComponentBody_SceneEnteredFunction final {void operator()(void* systemPtr, void* componentPool,Scene& scene) const {
	auto& pool = (*static_cast<ECSComponentPool<Entity, ComponentBody>*>(componentPool)).data();
    for (auto& component : pool) { 
        if (component.m_Physics) {
            component.addPhysicsToWorld(true);
        } 
    }
}};
struct priv::ComponentBody_SceneLeftFunction final {void operator()(void* systemPtr, void* componentPool, Scene& scene) const {
	auto& pool = (*static_cast<ECSComponentPool<Entity, ComponentBody>*>(componentPool)).data();
    for (auto& component : pool) { 
        if (component.m_Physics) {
            component.removePhysicsFromWorld(true);
        } 
    }
}};
    
ComponentBody_System_CI::ComponentBody_System_CI() {
    setUpdateFunction(ComponentBody_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentBody_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentBody_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentBody_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentBody_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentBody_SceneLeftFunction());
}

#pragma endregion


#pragma region System

void Engine::priv::ComponentBody_System::ParentChildVector::resize(size_t size) {
    Parents.resize(size, 0U);
    Order.resize(size, 0U);
    WorldTransforms.resize(size, IDENTITY_MATRIX);
    LocalTransforms.resize(size, IDENTITY_MATRIX);
}
void Engine::priv::ComponentBody_System::ParentChildVector::reserve(size_t size) {
    Parents.reserve(size);
    Order.reserve(size);
    WorldTransforms.reserve(size);
    LocalTransforms.reserve(size);
}
void Engine::priv::ComponentBody_System::ParentChildVector::reserve_from_insert(std::uint32_t parentID, std::uint32_t childID) {
    if (Parents.capacity() < parentID || Parents.capacity() < childID) {
        reserve(std::max(parentID, childID) + 50U);
    }
    if (Parents.size() < parentID || Parents.size() < childID) {
        resize(std::max(parentID, childID));
    }
}
void Engine::priv::ComponentBody_System::ParentChildVector::insert(std::uint32_t parentID, std::uint32_t childID) {
    reserve_from_insert(parentID, childID);
    if (getParent(childID) == parentID) {
        //std::cout << parentID << ", " << childID << " - added: already added\n";
        return;
    }

    //std::cout << parentID << ", " << childID << " - adding\n";
    bool added = false;
    for (size_t i = 0; i < Order.size(); ++i) {
        const auto& entityID = Order[i];
        if (entityID == parentID) {
            //insert after the parent node where the next available spot is
            //the next available spot is either the next zero or the next spot where that spot's parent is zero
            for (size_t j = i + 1; j < Order.size(); ++j) {
                const auto& entityIDCaseOne = Order[j];
                if (entityIDCaseOne == 0 || Parents[entityIDCaseOne - 1U] == 0) {
                    Order.insert(Order.begin() + j, childID);
                    Order.pop_back();
                    ++OrderHead;
                    added = true;
                    goto END_LOOP;
                }
            }
        }else if (entityID == childID) {
            //insert right before the child node
            Order.insert(Order.begin() + i, parentID);
            Order.pop_back();
            ++OrderHead;
            added = true;
            break;
        }else if (entityID == 0) {
            break;
        }
    }
    END_LOOP:

    if (!added) {
        /* add both at order head */
        Order[OrderHead]       = parentID;
        Order[OrderHead + 1U]  = childID;
        OrderHead             += 2;
    }
    getParent(childID) = parentID;

    getWorld(childID) = getWorld(parentID) * getLocal(childID);
}
void Engine::priv::ComponentBody_System::ParentChildVector::remove(std::uint32_t parentID, std::uint32_t childID) {
    if (getParent(childID) == 0) {
        //std::cout << parentID << ", " << childID << " - remove: already removed\n";
        return;
    }
    size_t parentIndex = 0;
    size_t erasedIndex = 0;
    bool foundParent   = false;

    for (size_t i = 0; i < Order.size(); ++i) {
        const auto& entityID = Order[i];
        if (entityID == 0) {
            break;
        }else if (entityID == parentID) {
            parentIndex = i;
            foundParent = true;
            break;
        }
    }
    if (!foundParent) {
        //std::cout << parentID << ", " << childID << " - remove: not found\n";
        return;
    }
    getParent(childID) = 0;
    erasedIndex = parentIndex;
    //std::cout << parentID << ", " << childID << " - removing\n";
    for (size_t i = parentIndex; i < Order.size(); ++i) {
        const auto& entityID = Order[i];
        if (entityID == childID) {
            erasedIndex = i;
            Order[i] = 0;

            //now move all children of parent to be next to parent
            for (size_t j = i + 1U; j < Order.size(); ++j) {
                const auto& entityIDCaseOne = Order[j];
                if (Order[j] == 0) {
                    break;
                }
                if (getParent(entityIDCaseOne) == parentID) {
                    std::swap(Order[j - 1U], Order[j]);
                    ++erasedIndex;
                }else if (getParent(entityIDCaseOne) == childID && Order[j - 1U] == 0) {
                    Order[j - 1U] = childID;
                }else if (getParent(entityIDCaseOne) == 0) {
                    break;
                }
            }
            //cleanup / edge cases
            //if (Order[erasedIndex + 1U] == 0) {
            if (Order[erasedIndex] == 0) {
                Order.erase(Order.begin() + erasedIndex);
                Order.emplace_back(0);
                --OrderHead;
            }
            if (parentIndex > 0 && Order[parentIndex + 1U] == 0) {
                Order[parentIndex] = 0;
                Order.erase(Order.begin() + parentIndex);
                Order.emplace_back(0);
                --OrderHead;
            }
            break;
        }
    }
}
std::uint32_t Engine::priv::ComponentBody_System::ParentChildVector::size() const {
    return OrderHead;
}
size_t Engine::priv::ComponentBody_System::ParentChildVector::capacity() const {
    return Order.capacity();
}

Engine::priv::ComponentBody_System::ComponentBody_System(const Engine::priv::ECSSystemCI& systemCI, Engine::priv::ECS<Entity>& ecs) : Engine::priv::ECSSystem<Entity, ComponentBody>(systemCI, ecs) {

}
Engine::priv::ComponentBody_System::~ComponentBody_System() {

}


#pragma endregion
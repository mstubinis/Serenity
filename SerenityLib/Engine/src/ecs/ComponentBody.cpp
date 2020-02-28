#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/system/Engine.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/physics/Collision.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <BulletCollision/Gimpact/btGImpactShape.h>


#include <iostream>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region CollisionCallbackParticipant
CollisionCallbackEventData::CollisionCallbackEventData(ComponentBody& a, ComponentBody& b, glm::vec3& c, glm::vec3& d, glm::vec3& e, glm::vec3& f, glm::vec3& g, glm::vec3& h) :
ownerBody(a), otherBody(b), ownerHit(c), otherHit(d), normalOnB(e), ownerLocalHit(f), otherLocalHit(g), normalFromAB(h) {
    ownerCollisionObj       = nullptr;
    otherCollisionObj       = nullptr;
    ownerModelInstanceIndex = 0;
    otherModelInstanceIndex = 0;
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
    //modelMatrix    = std::move(other.modelMatrix);
    linearVelocity = std::move(other.linearVelocity);
}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(ComponentBody::NormalData&& other) noexcept {
    //move assignment
    if (&other != this) {
        position       = std::move(other.position);
        rotation       = std::move(other.rotation);
        scale          = std::move(other.scale);
        //modelMatrix    = std::move(other.modelMatrix);
        linearVelocity = std::move(other.linearVelocity);
    }
    return *this;
}


#pragma endregion

#pragma region Component

ComponentBody::ComponentBody(const Entity& entity) : ComponentBaseClass(entity) {
    m_Physics                 = false;
    data.p                    = nullptr;
    data.n                    = NEW NormalData();
    auto& normalData          = *data.n;
    Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
}
ComponentBody::ComponentBody(const Entity& entity, const CollisionType::Type collisionType) : ComponentBaseClass(entity) {
    m_Physics               = true;
    data.n                  = nullptr;
    data.p                  = NEW PhysicsData();

    data.p->bullet_motionState = btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    setCollision(collisionType, 1.0f);
    rebuildRigidBody(false);
}
ComponentBody::~ComponentBody() {
    //destructor
    if (m_Physics) {
        SAFE_DELETE(data.p);
    }else{
        SAFE_DELETE(data.n);
    }
}
ComponentBody::ComponentBody(ComponentBody&& other) noexcept {
    //move constructor
    using std::swap;
    m_Physics          = std::move(other.m_Physics);
    m_Forward          = std::move(other.m_Forward);
    m_Right            = std::move(other.m_Right);
    m_Up               = std::move(other.m_Up);
    swap(m_Owner,other.m_Owner);
    m_CollisionFunctor.swap(other.m_CollisionFunctor);
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
    if (other.m_Physics) {
        data.p         = std::exchange(other.data.p, nullptr);
    }else{
        data.n         = std::exchange(other.data.n, nullptr);
    }
    setInternalPhysicsUserPointer(this);
}
ComponentBody& ComponentBody::operator=(ComponentBody&& other) noexcept {
    //move assignment
    if (&other != this) {
        using std::swap;
        m_Physics          = std::move(other.m_Physics);
        m_Forward          = std::move(other.m_Forward);
        m_Right            = std::move(other.m_Right);
        m_Up               = std::move(other.m_Up);
        swap(m_Owner,other.m_Owner);
        m_CollisionFunctor.swap(other.m_CollisionFunctor);
        m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
        if (other.m_Physics) {
            SAFE_DELETE(data.p);
            data.p         = std::exchange(other.data.p, nullptr);
        }else{
            SAFE_DELETE(data.n);
            data.n         = std::exchange(other.data.n, nullptr);
        }
        setInternalPhysicsUserPointer(this);
    }
    return *this;
}

void ComponentBody::onEvent(const Event& _event) {

}

void ComponentBody::rebuildRigidBody(const bool addBodyToPhysicsWorld, const bool threadSafe) {
    if (m_Physics) {
        auto& inertia = data.p->collision->getBtInertia();
        auto* shape = data.p->collision->getBtShape();
        if (shape) {
            if (data.p->bullet_rigidBody) {
                removePhysicsFromWorld(true, threadSafe);
                SAFE_DELETE(data.p->bullet_rigidBody);
            }
            btRigidBody::btRigidBodyConstructionInfo CI(data.p->mass, &data.p->bullet_motionState, shape, inertia);
            data.p->bullet_rigidBody = new btRigidBody(CI);
            data.p->bullet_rigidBody->setSleepingThresholds(0.015f, 0.015f);
            data.p->bullet_rigidBody->setFriction(0.3f);
            setDamping(static_cast<decimal>(0.1), static_cast<decimal>(0.4));
            data.p->bullet_rigidBody->setMassProps(static_cast<btScalar>(data.p->mass), inertia);
            data.p->bullet_rigidBody->updateInertiaTensor();
            setInternalPhysicsUserPointer(this);
            if (addBodyToPhysicsWorld)
                addPhysicsToWorld(true, threadSafe);
        }
    }
}
//kinda ugly
void ComponentBody::setInternalPhysicsUserPointer(void* userPtr) {
    if (m_Physics) {
        if (data.p->bullet_rigidBody) {
            data.p->bullet_rigidBody->setUserPointer(userPtr);
            auto* shape = data.p->bullet_rigidBody->getCollisionShape();
            if (shape) {
                shape->setUserPointer(userPtr);
            }
        }
    }
}
void ComponentBody::removePhysicsFromWorld(const bool force, const bool threadSafe) {
    if (force) {    
        data.p->forcedOut = true;
    }
    if(threadSafe)
        Physics::removeRigidBodyThreadSafe(*this);
    else
        Physics::removeRigidBody(*this);
}
void ComponentBody::addPhysicsToWorld(const bool force, const bool threadSafe) {
    if (!force && data.p->forcedOut)
        return;
    if(threadSafe)
        Physics::addRigidBodyThreadSafe(*this);
    else
        Physics::addRigidBody(*this);
    data.p->forcedOut = false;
}
const bool ComponentBody::hasPhysics() const {
    return m_Physics;
}
void ComponentBody::setUserPointer(void* userPtr) {
    m_UserPointer  = userPtr;
}
void ComponentBody::setUserPointer1(void* userPtr) {
    m_UserPointer1 = userPtr;
}
void ComponentBody::setUserPointer2(void* userPtr) {
    m_UserPointer2 = userPtr;
}
void* ComponentBody::getUserPointer() const {
    return m_UserPointer;
}
void* ComponentBody::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentBody::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentBody::collisionResponse(CollisionCallbackEventData& data) const {
    //if(m_CollisionFunctor)
    m_CollisionFunctor( std::ref(data) );
}
const ushort ComponentBody::getCollisionGroup() const {
    if (m_Physics) {
        return data.p->group;
    }
    return static_cast<ushort>(0);
}
const ushort ComponentBody::getCollisionMask() const {
    if (m_Physics) {
        return data.p->mask;
    }
    return static_cast<ushort>(0);
}
const ushort ComponentBody::getCollisionFlags() const {
    if (m_Physics) {
        return data.p->bullet_rigidBody->getCollisionFlags();
    }
    return static_cast<ushort>(0);
}

const decimal ComponentBody::getDistance(const Entity& p_Other) const {
    const auto other_position = p_Other.getComponent<ComponentBody>()->position();
    return glm::distance(position(), other_position);
}
const unsigned long long ComponentBody::getDistanceLL(const Entity& p_Other) const {
    const auto other_position = p_Other.getComponent<ComponentBody>()->position();
    return static_cast<unsigned long long>(glm::distance(position(), other_position));
}
void ComponentBody::alignTo(const glm_vec3& p_Direction) {
    if (m_Physics) {
        //recheck this
        btTransform tr;
        data.p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        Math::recalculateForwardRightUp(*data.p->bullet_rigidBody, m_Forward, m_Right, m_Up);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(normalData.rotation, p_Direction);
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
Collision* ComponentBody::getCollision() const {
    if (m_Physics) {
        return data.p->collision;
    }
    return nullptr;
}
void ComponentBody::setCollision(const CollisionType::Type p_CollisionType, const float p_Mass) {
    if (!data.p->collision) { //TODO: clean this up, its hacky and evil. its being used on the ComponentBody_EntityAddedToSceneFunction
        auto* modelComponent = m_Owner.getComponent<ComponentModel>();
        if (modelComponent) {
            if (p_CollisionType == CollisionType::Compound) {
                data.p->collision = NEW Collision(*this, *modelComponent, p_Mass);
            }else{
                data.p->collision = NEW Collision(*this, p_CollisionType, &modelComponent->getModel(), p_Mass);
            }
        }else{
            data.p->collision = NEW Collision(*this, p_CollisionType, nullptr, p_Mass);
        }
    }
    data.p->mass = p_Mass;
    data.p->collision->setMass(data.p->mass);
    if (data.p->bullet_rigidBody) {
        data.p->bullet_rigidBody->setCollisionShape(data.p->collision->getBtShape());
        data.p->bullet_rigidBody->setMassProps(static_cast<btScalar>(data.p->mass), data.p->collision->getBtInertia());
        data.p->bullet_rigidBody->updateInertiaTensor();
    }
    setInternalPhysicsUserPointer(this);
}
//double check this...
void ComponentBody::setCollision(Collision* p_Collision) {
    if (data.p->collision) {
        removePhysicsFromWorld(false, false);
        SAFE_DELETE(data.p->collision);
    }
    data.p->collision = p_Collision;
    if (data.p->bullet_rigidBody) {
        data.p->bullet_rigidBody->setCollisionShape(data.p->collision->getBtShape());
        data.p->bullet_rigidBody->setMassProps(static_cast<btScalar>(data.p->mass), data.p->collision->getBtInertia());
        data.p->bullet_rigidBody->updateInertiaTensor();
        addPhysicsToWorld(false, false);
    }
    setInternalPhysicsUserPointer(this);
}

void ComponentBody::translate(const glm_vec3& p_Translation, const bool p_Local) {
	ComponentBody::translate(p_Translation.x, p_Translation.y, p_Translation.z, p_Local);
}
void ComponentBody::translate(const decimal& p_Translation, const bool p_Local) {
	ComponentBody::translate(p_Translation, p_Translation, p_Translation, p_Local);
}
void ComponentBody::translate(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
        data.p->bullet_rigidBody->activate();
        btVector3 v(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        Math::translate(*data.p->bullet_rigidBody, v, p_Local);
        ComponentBody::setPosition(position() + Engine::Math::btVectorToGLM(v));
    }else{
        auto& normalData = *data.n;
        glm_vec3 offset(p_X, p_Y, p_Z);
        if (p_Local) {
            offset = normalData.rotation * offset;
        }
		ComponentBody::setPosition(normalData.position + offset);
    }
}
void ComponentBody::rotate(const glm_vec3& p_Rotation, const bool p_Local) {
	ComponentBody::rotate(p_Rotation.x, p_Rotation.y, p_Rotation.z, p_Local);
}
void ComponentBody::rotate(const decimal& p_Pitch, const decimal& p_Yaw, const decimal& p_Roll, const bool p_Local) {
    if (m_Physics) {
        //if (!data.p->bullet_rigidBody) {
        //    return;
        //}
        auto& bt_rigidBody = *data.p->bullet_rigidBody;
        btQuaternion quat = bt_rigidBody.getWorldTransform().getRotation().normalize();
        glm_quat glmquat(quat.w(), quat.x(), quat.y(), quat.z());
        Math::rotate(glmquat, p_Pitch, p_Yaw, p_Roll);
        quat = btQuaternion(static_cast<btScalar>(glmquat.x), static_cast<btScalar>(glmquat.y), static_cast<btScalar>(glmquat.z), static_cast<btScalar>(glmquat.w));
        bt_rigidBody.getWorldTransform().setRotation(quat);
        Math::recalculateForwardRightUp(bt_rigidBody, m_Forward, m_Right, m_Up);
    }else{
        auto& normalData = *data.n;
        Math::rotate(normalData.rotation, p_Pitch, p_Yaw, p_Roll);
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::scale(const glm_vec3& p_ScaleAmount) { 
	ComponentBody::scale(p_ScaleAmount.x, p_ScaleAmount.y, p_ScaleAmount.z);
}
void ComponentBody::scale(const decimal& p_ScaleAmount) {
	ComponentBody::scale(p_ScaleAmount, p_ScaleAmount, p_ScaleAmount);
}
void ComponentBody::scale(const decimal& p_X, const decimal& p_Y, const decimal& p_Z) {
    if (m_Physics) {
        const auto& newScale = btVector3(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        const auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        auto collisionShape = collision_.getBtShape();
        if (collisionShape) {
            if (collision_.getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collisionShape);
                if (compoundShapeCast) {
                    const auto numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                auto& _convexHullCast = *convexHullCast;
                                const auto& _scl = _convexHullCast.getLocalScaling() + newScale;
                                _convexHullCast.setLocalScaling(_scl);
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                auto _triHullCast = *triHullCast;
                                const auto& _scl = _triHullCast.getLocalScaling() + newScale;
                                _triHullCast.setLocalScaling(_scl);
                            }
                        }
                    }
                }
            }else if (collision_.getType() == CollisionType::ConvexHull) {
                btUniformScalingShape* convex = static_cast<btUniformScalingShape*>(collisionShape);
                const auto& _scl = convex->getLocalScaling() + newScale;
                convex->setLocalScaling(_scl);
            }else if (collision_.getType() == CollisionType::TriangleShapeStatic) {
                btScaledBvhTriangleMeshShape* tri = static_cast<btScaledBvhTriangleMeshShape*>(collisionShape);
                const auto& _scl = tri->getLocalScaling() + newScale;
                tri->setLocalScaling(_scl);
            }else if (collision_.getType() == CollisionType::Sphere) {
                btMultiSphereShape* sph = static_cast<btMultiSphereShape*>(collisionShape);
                const auto& _scl = sph->getLocalScaling() + newScale;
                sph->setLocalScaling(_scl);
                sph->setImplicitShapeDimensions(_scl);
                sph->recalcLocalAabb();
            }else if (collision_.getType() == CollisionType::TriangleShape) {
                btGImpactMeshShape* gImpact = static_cast<btGImpactMeshShape*>(collisionShape);
                const auto& _scl = gImpact->getLocalScaling() + newScale;
                gImpact->setLocalScaling(_scl);
                gImpact->updateBound();
                gImpact->postUpdate();
            }
        }
    }else{
        auto& normalData = *data.n;
        auto& scale_ = normalData.scale;
		scale_.x += p_X;
		scale_.y += p_Y;
		scale_.z += p_Z;
    }
    auto* models = m_Owner.getComponent<ComponentModel>();
    if (models) {
        ComponentModel_Functions::CalculateRadius(*models);
    }
}
void ComponentBody::setPosition(const glm_vec3& p_NewPosition) {
	ComponentBody::setPosition(p_NewPosition.x, p_NewPosition.y, p_NewPosition.z);
}
void ComponentBody::setPosition(const decimal& p_NewPosition) {
	ComponentBody::setPosition(p_NewPosition, p_NewPosition, p_NewPosition);
}
void ComponentBody::setPosition(const decimal& p_X, const decimal& p_Y, const decimal& p_Z) {
    if (m_Physics) {
        btTransform tr;
        tr.setOrigin(btVector3(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z)));
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

        auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
        auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());

        const auto entityIndex = m_Owner.id() - 1U;

		position_.x         = p_X;
		position_.y         = p_Y;
		position_.z         = p_Z;

        //normalData.modelMatrix[3][0] = p_X;
        //normalData.modelMatrix[3][1] = p_Y;
        //normalData.modelMatrix[3][2] = p_Z;


        auto& localMatrix = system.ParentChildSystem.LocalTransforms[entityIndex];
        localMatrix[3][0] = p_X;
        localMatrix[3][1] = p_Y;
        localMatrix[3][2] = p_Z;

        auto& worldMatrix = system.ParentChildSystem.WorldTransforms[entityIndex];
        worldMatrix[3][0] = p_X;
        worldMatrix[3][1] = p_Y;
        worldMatrix[3][2] = p_Z;
    }
}
void ComponentBody::setGravity(const decimal& p_X, const decimal& p_Y, const decimal& p_Z) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        physicsData.bullet_rigidBody->setGravity(btVector3(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z)));
    }
}
void ComponentBody::setRotation(const glm_quat& p_NewRotation) {
	ComponentBody::setRotation(p_NewRotation.x, p_NewRotation.y, p_NewRotation.z, p_NewRotation.w);
}
void ComponentBody::setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        btQuaternion quat(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z), static_cast<btScalar>(w));
        quat = quat.normalize();
        auto& rigidBody = *physicsData.bullet_rigidBody;
        btTransform tr; tr.setOrigin(rigidBody.getWorldTransform().getOrigin());
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
void ComponentBody::setScale(const glm_vec3& p_NewScale) {
	ComponentBody::setScale(p_NewScale.x, p_NewScale.y, p_NewScale.z);
}
void ComponentBody::setScale(const decimal& p_NewScale) {
	ComponentBody::setScale(p_NewScale, p_NewScale, p_NewScale);
}
void ComponentBody::setScale(const decimal& p_X, const decimal& p_Y, const decimal& p_Z) {
    if (m_Physics) {
        const auto  newScale = btVector3(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        const auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        auto collisionShape = collision_.getBtShape();
        if (collisionShape) {
            if (collision_.getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collisionShape);
                if (compoundShapeCast) {
                    const int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                convexHullCast->setLocalScaling(newScale);
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                triHullCast->setLocalScaling(newScale);
                            }
                        }
                    }
                }
            }else if (collision_.getType() == CollisionType::ConvexHull) {
                btUniformScalingShape* convex = static_cast<btUniformScalingShape*>(collisionShape);
                convex->setLocalScaling(newScale);
            }else if (collision_.getType() == CollisionType::TriangleShapeStatic) {
                btScaledBvhTriangleMeshShape* tri = static_cast<btScaledBvhTriangleMeshShape*>(collisionShape);
                tri->setLocalScaling(newScale);
            }else if (collision_.getType() == CollisionType::Sphere) {
                btMultiSphereShape* sph = static_cast<btMultiSphereShape*>(collisionShape);
                sph->setLocalScaling(newScale);
                sph->setImplicitShapeDimensions(newScale);
                sph->recalcLocalAabb();
            }else if (collision_.getType() == CollisionType::TriangleShape) {
                btGImpactMeshShape* gImpact = static_cast<btGImpactMeshShape*>(collisionShape);
                gImpact->setLocalScaling(newScale);
                gImpact->updateBound();
                gImpact->postUpdate();
            }
        }
    }else{
        auto& normalData = *data.n;
        auto& scale = normalData.scale;
        scale.x = p_X;
		scale.y = p_Y;
		scale.z = p_Z;
    }
    auto* models = m_Owner.getComponent<ComponentModel>();
    if (models) {
        priv::ComponentModel_Functions::CalculateRadius(*models);
    }
}

const glm_vec3 ComponentBody::localPosition() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        return Math::btVectorToGLM(tr.getOrigin());
    }
    return data.n->position;
}

const glm_vec3 ComponentBody::position() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        return Math::btVectorToGLM(tr.getOrigin());
    }
    //const auto& modelMatrix_ = data.n->modelMatrix;
    //return Math::getMatrixPosition(modelMatrix_);

    auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
    //const auto& matrix = system.ParentChildSystem.LocalTransforms[m_Owner.id() - 1U];
    const auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return Math::getMatrixPosition(matrix);
}
const glm::vec3 ComponentBody::position_render() const { //theres prob a better way to do this
    if (m_Physics) {
        auto tr = data.p->bullet_rigidBody->getWorldTransform();
        return Math::btVectorToGLM(tr.getOrigin());
    }
    //const auto& modelMatrix_ = data.n->modelMatrix;
    //return glm::vec3(modelMatrix_[3][0], modelMatrix_[3][1], modelMatrix_[3][2]);

    auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
    //const auto& matrix = system.ParentChildSystem.LocalTransforms[m_Owner.id() - 1U];
    const auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return Math::getMatrixPosition(matrix);
}
const glm::vec3 ComponentBody::getScreenCoordinates(const bool p_ClampToEdge) const {
	return Math::getScreenCoordinates(position(), *m_Owner.scene().getActiveCamera(), p_ClampToEdge);
}
const ScreenBoxCoordinates ComponentBody::getScreenBoxCoordinates(const float p_MinOffset) const {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = position();
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
    const auto  testRes          = Math::getScreenCoordinates(worldPos + (camvectest * static_cast<decimal>(radius)), camera, false);
    const auto test              = glm::vec2(testRes.x, testRes.y);
    const auto radius2D          = glm::max(p_MinOffset, glm::distance(test, center2D));
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
const glm_vec3 ComponentBody::getScale() const {
    if (m_Physics) {
        const auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        auto collisionShape = collision_.getBtShape();
        if (collisionShape) {
            if (collision_.getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collisionShape);
                if (compoundShapeCast) {
                    const int& numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                const auto ret = Math::btVectorToGLM(convexHullCast->getLocalScaling());
                                return ret;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                const auto ret = Math::btVectorToGLM(triHullCast->getLocalScaling());
                                return ret;
                            }
                        }
                    }
                }
            }else if (collision_.getType() == CollisionType::ConvexHull) {
                btUniformScalingShape* convex = static_cast<btUniformScalingShape*>(collisionShape);
                if (convex) {
                    const auto ret = Math::btVectorToGLM(convex->getLocalScaling());
                    return ret;
                }
            }else if (collision_.getType() == CollisionType::TriangleShapeStatic) {
                btScaledBvhTriangleMeshShape* tri = static_cast<btScaledBvhTriangleMeshShape*>(collisionShape);
                if (tri) {
                    const auto ret = Math::btVectorToGLM(tri->getLocalScaling());
                    return ret;
                }
            }else if (collision_.getType() == CollisionType::Sphere) {
                btMultiSphereShape* sph = static_cast<btMultiSphereShape*>(collisionShape);
                if (sph) {
                    const auto ret = Math::btVectorToGLM(sph->getLocalScaling());
                    return ret;
                }
            }else if (collision_.getType() == CollisionType::TriangleShape) {
                btGImpactMeshShape* gImpact = static_cast<btGImpactMeshShape*>(collisionShape);
                if (gImpact) {
                    const auto ret = Math::btVectorToGLM(gImpact->getLocalScaling());
                    return ret;
                }
            }
        }
        return glm_vec3(static_cast<decimal>(1.0));
    }
    return data.n->scale;
}
const glm_quat ComponentBody::rotation() const {
    if (m_Physics) {
        btTransform tr;
        data.p->bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        auto quat = tr.getRotation();
        return Engine::Math::btToGLMQuat(quat);
    }
    return data.n->rotation;
}

const glm_vec3& ComponentBody::forward() const {
	return m_Forward; 
}
const glm_vec3& ComponentBody::right() const {
	return m_Right; 
}
const glm_vec3& ComponentBody::up() const {
	return m_Up; 
}
const glm_vec3 ComponentBody::getLinearVelocity() const  {
    if (m_Physics) {
        const btVector3& v = data.p->bullet_rigidBody->getLinearVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    auto& normalData = *data.n;
    return normalData.linearVelocity;
}
const glm_vec3 ComponentBody::getAngularVelocity() const  {
    if (m_Physics) {
        const btVector3& v = data.p->bullet_rigidBody->getAngularVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    return glm_vec3(static_cast<decimal>(0.0));
}
const float ComponentBody::mass() const {
	return data.p->mass; 
}
const glm_mat4 ComponentBody::modelMatrix() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
#ifndef BT_USE_DOUBLE_PRECISION
        glm::mat4 modelMatrix_(1.0f);
#else
        glm_mat4 modelMatrix_(static_cast<decimal>(1.0));
#endif
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        btScalar* val_ptr = (btScalar*)glm::value_ptr(modelMatrix_);
        tr.getOpenGLMatrix(val_ptr);
        auto& collision_ = *physicsData.collision;
        if (collision_.getBtShape()) {
            const auto scale = getScale();
            modelMatrix_ = glm::scale(modelMatrix_, scale);
        }
        return modelMatrix_;
    }
    //return data.n->modelMatrix;

    auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
    //const auto& matrix = system.ParentChildSystem.LocalTransforms[m_Owner.id() - 1U];
    const auto& matrix = system.ParentChildSystem.WorldTransforms[m_Owner.id() - 1U];
    return matrix;
}
const glm::mat4 ComponentBody::modelMatrixRendering() const {
    glm::mat4 ret = static_cast<glm::mat4>(modelMatrix());
    return ret;
}
const btRigidBody& ComponentBody::getBtBody() const {
	return *data.p->bullet_rigidBody;
}
void ComponentBody::setDamping(const decimal& p_LinearFactor, const decimal& p_AngularFactor) {
	data.p->bullet_rigidBody->setDamping(static_cast<btScalar>(p_LinearFactor), static_cast<btScalar>(p_AngularFactor));
}
void ComponentBody::setCollisionGroup(const short group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != group) {
            removePhysicsFromWorld(false);
            phyData.group = group;
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::setCollisionMask(const short mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != mask) {
            removePhysicsFromWorld(false);
            phyData.mask = mask;
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::setCollisionGroup(const CollisionFilter::Filter group) {
    ComponentBody::setCollisionGroup(static_cast<short>(group));
}
void ComponentBody::setCollisionMask(const CollisionFilter::Filter mask) {
    ComponentBody::setCollisionMask(static_cast<short>(mask));
}
void ComponentBody::addCollisionGroup(const short group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != (phyData.group | group)) {
            removePhysicsFromWorld(false);
            phyData.group = phyData.group | group;
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::addCollisionMask(const short mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != (phyData.mask | mask)) {
            removePhysicsFromWorld(false);
            phyData.mask = phyData.mask | mask;
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::addCollisionGroup(const CollisionFilter::Filter group) {
    ComponentBody::addCollisionGroup(static_cast<short>(group));
}
void ComponentBody::addCollisionMask(const CollisionFilter::Filter mask) {
    ComponentBody::addCollisionMask(static_cast<short>(mask));
}
void ComponentBody::setCollisionFlag(const short flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != flag) {
            removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(flag);
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::setCollisionFlag(const CollisionFlag::Flag flag) {
    ComponentBody::setCollisionFlag(static_cast<short>(flag));
}
void ComponentBody::addCollisionFlag(const short flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != (currFlags | flag)) {
            removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(currFlags | flag);
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::addCollisionFlag(const CollisionFlag::Flag flag) {
    ComponentBody::addCollisionFlag(static_cast<short>(flag));
}
void ComponentBody::removeCollisionGroup(const short group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != (phyData.group & ~group)) {
            removePhysicsFromWorld(false);
            phyData.group = phyData.group & ~group;
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::removeCollisionMask(const short mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != (phyData.mask & ~mask)) {
            removePhysicsFromWorld(false);
            phyData.mask = phyData.mask & ~mask;
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::removeCollisionFlag(const short flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != (currFlags & ~flag)) {
            removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(currFlags & ~flag);
            addPhysicsToWorld(false);
        }
    }
}
void ComponentBody::removeCollisionGroup(const CollisionFilter::Filter group) {
    ComponentBody::removeCollisionGroup(static_cast<short>(group));
}
void ComponentBody::removeCollisionMask(const CollisionFilter::Filter mask) {
    ComponentBody::removeCollisionMask(static_cast<short>(mask));
}
void ComponentBody::removeCollisionFlag(const CollisionFlag::Flag flag) {
    ComponentBody::removeCollisionFlag(static_cast<short>(flag));
}


//TODO: reconsider how this works
void ComponentBody::setDynamic(const bool dynamic) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        auto& rigidBody = *physicsData.bullet_rigidBody;
        if (dynamic) {
            removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            addPhysicsToWorld(false);
            rigidBody.activate();
        }else{
            removePhysicsFromWorld(false);
            rigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            addPhysicsToWorld(false);
            rigidBody.activate();
        }
    }
}
void ComponentBody::setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
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
void ComponentBody::setLinearVelocity(const glm_vec3& velocity, const bool local) {
	ComponentBody::setLinearVelocity(velocity.x, velocity.y, velocity.z, local);
}
void ComponentBody::setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    if (m_Physics) {
		auto& rigidBody = *data.p->bullet_rigidBody;
		rigidBody.activate();
		btVector3 v(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
		Math::translate(rigidBody, v, local);
		rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setAngularVelocity(const glm_vec3& velocity, const bool local) {
	ComponentBody::setAngularVelocity(velocity.x, velocity.y, velocity.z, local);
}
void ComponentBody::applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyCentralForce(v);
    }
}
void ComponentBody::applyForce(const glm_vec3& force, const glm_vec3& origin, const bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(force.x), static_cast<btScalar>(force.y), static_cast<btScalar>(force.z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyForce(v, btVector3(static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z)));
    }
}
void ComponentBody::applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, const bool local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(impulse.x), static_cast<btScalar>(impulse.y), static_cast<btScalar>(impulse.z));
        Math::translate(rigidBody, v, local);
        rigidBody.applyImpulse(v, btVector3(static_cast<btScalar>(origin.x), static_cast<btScalar>(origin.y), static_cast<btScalar>(origin.z)));
    }
}
void ComponentBody::applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local) {
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
void ComponentBody::applyTorque(const glm_vec3& torque, const bool local) {
	ComponentBody::applyTorque(torque.x, torque.y, torque.z, local);
}
void ComponentBody::applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local) {
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
void ComponentBody::applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local) {
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
void ComponentBody::setMass(const float mass) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        physicsData.mass = mass;
        Collision& collision = *physicsData.collision;
        if (collision.getBtShape()) {
            collision.setMass(physicsData.mass);
            if (physicsData.bullet_rigidBody) {
                physicsData.bullet_rigidBody->setMassProps(static_cast<btScalar>(physicsData.mass), collision.getBtInertia());
            }
        }
    }
}
void ComponentBody::addChild(const Entity& child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
        auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
        auto& pcs = system.ParentChildSystem;
        pcs.insert(m_Owner.id(), child.id());
    }
}
void ComponentBody::addChild(const ComponentBody& child) const {
    ComponentBody::addChild(child.m_Owner);
}
void ComponentBody::removeChild(const Entity& child) const {
    if (child.sceneID() == m_Owner.sceneID()) {
        auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
        auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
        auto& pcs = system.ParentChildSystem;
        pcs.remove(m_Owner.id(), child.id());
    }
}
void ComponentBody::removeChild(const ComponentBody& child) const {
    ComponentBody::removeChild(child.m_Owner);
}
const bool ComponentBody::hasParent() const {
    auto& ecs = Engine::priv::InternalScenePublicInterface::GetECS(m_Owner.scene());
    auto& system = static_cast<Engine::priv::ComponentBody_System&>(ecs.getSystem<ComponentBody>());
    auto& pcs = system.ParentChildSystem;
    return (pcs.Parents[m_Owner.id() - 1U] > 0);
}


#pragma endregion

#pragma region SystemCI

constexpr glm_mat4 IDENTITY_MATRIX = glm_mat4(1.0);

struct priv::ComponentBody_UpdateFunction final { void operator()(void* systemPtr, void* componentPool, const float dt, Scene& scene) const {
    auto& system            = *static_cast<Engine::priv::ComponentBody_System*>(systemPtr);
    auto& pool              = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(componentPool);
    auto& components        = pool.data();
    const decimal double_dt = static_cast<decimal>(dt);

    auto lamda_update_component = [double_dt, &system](ComponentBody& b, const size_t& i) {
        const auto entityIndex = b.m_Owner.id() - 1U;
        auto& localMatrix = system.ParentChildSystem.LocalTransforms[entityIndex];
        auto& worldMatrix = system.ParentChildSystem.WorldTransforms[entityIndex];
        if (b.m_Physics) {
            auto& rigidBody = *b.data.p->bullet_rigidBody;
            Engine::Math::recalculateForwardRightUp(rigidBody, b.m_Forward, b.m_Right, b.m_Up);

            localMatrix = b.modelMatrix();
            worldMatrix = localMatrix;
        }else{
            auto& n = *b.data.n;
            n.position += (n.linearVelocity * double_dt);
            //n.modelMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);

            localMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);
            worldMatrix = localMatrix;
        }
    };

    if (components.size() < 200) {
        for (size_t i = 0; i < components.size(); ++i) {
            lamda_update_component(components[i], i);
        }
    }else{
        priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_component, components, true);
    }
    
    auto& pcs = system.ParentChildSystem;
    for (size_t i = 0; i < pcs.Order.size(); ++i) {
        const unsigned int entityID = pcs.Order[i];
        if (entityID > 0) {
            const unsigned int entityIndex = entityID - 1U;
            const unsigned int parentID = pcs.Parents[entityIndex];
            if (parentID == 0) {
                pcs.WorldTransforms[entityIndex] = pcs.LocalTransforms[entityIndex];
            }else{
                const unsigned int parentIndex = parentID - 1U;
                pcs.WorldTransforms[entityIndex] = pcs.WorldTransforms[parentIndex] * pcs.LocalTransforms[entityIndex];
            }
        }else{
            break;
        }
    }   

#if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
    for (auto& componentBody : components) {
        auto* model = componentBody.getOwner().getComponent<ComponentModel>();
        if (model) {
            const auto world_pos = glm::vec3(componentBody.position());
            const auto world_rot = glm::quat(componentBody.rotation());
            const auto world_scl = glm::vec3(componentBody.getScale());
            for (size_t i = 0; i < model->getNumModels(); ++i) {
                auto& modelInstance = (*model)[i];

                const auto rotation = world_rot * modelInstance.orientation();
                const auto fwd      = glm::normalize(Math::getForward(rotation)) * 0.3f;
                const auto right    = glm::normalize(Math::getRight(rotation)) * 0.3f;
                const auto up       = glm::normalize(Math::getUp(rotation)) * 0.3f;

                auto& physics = Engine::priv::Core::m_Engine->m_PhysicsManager;
                physics.debug_draw_line(world_pos, (world_pos+fwd) /* * glm::length(world_scl) */, 1, 0, 0, 1);
                physics.debug_draw_line(world_pos, (world_pos+right) /* * glm::length(world_scl) */, 0, 1, 0, 1);
                physics.debug_draw_line(world_pos, (world_pos+up) /* * glm::length(world_scl) */, 0, 0, 1, 1);
            }
        }
    }
#endif

}};
struct priv::ComponentBody_ComponentAddedToEntityFunction final {void operator()(void* systemPtr, void* component, Entity& entity) const {
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
struct priv::ComponentBody_ComponentRemovedFromEntityFunction final { void operator()(void* systemPtr, Entity& entity) const {
    auto& system  = *static_cast<Engine::priv::ComponentBody_System*>(systemPtr);
    const auto id = entity.id();
    auto& pcs     = system.ParentChildSystem;

    if (pcs.Parents[id - 1U] > 0) {
        pcs.remove(pcs.Parents[id - 1U], id);
    }
}};
struct priv::ComponentBody_EntityAddedToSceneFunction final {void operator()(void* systemPtr, void* componentPool,Entity& entity, Scene& scene) const {
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

void Engine::priv::ComponentBody_System::ParentChildVector::resize(const size_t size) {
    Parents.resize(size, 0U);
    Order.resize(size, 0U);
    WorldTransforms.resize(size, IDENTITY_MATRIX);
    LocalTransforms.resize(size, IDENTITY_MATRIX);
}
void Engine::priv::ComponentBody_System::ParentChildVector::reserve(const size_t size) {
    Parents.reserve(size);
    Order.reserve(size);
    WorldTransforms.reserve(size);
    LocalTransforms.reserve(size);
}
void Engine::priv::ComponentBody_System::ParentChildVector::insert(const std::uint32_t parentID, const std::uint32_t childID) {
    //if (Parents.size() <= parentID || Parents.size() <= childID) {

    if (Parents.capacity() < parentID || Parents.capacity() < childID) {
        reserve(std::max(parentID, childID) + 50U);
    }
    if (Parents.size() < parentID || Parents.size() < childID) {
        resize(std::max(parentID, childID));
    }

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
    Parents[childID - 1U] = parentID;


    WorldTransforms[childID - 1U] = WorldTransforms[parentID - 1U] * LocalTransforms[childID - 1U];
}
void Engine::priv::ComponentBody_System::ParentChildVector::remove(const std::uint32_t parentID, const std::uint32_t childID) {
    size_t parentIndex    = 0;
    bool foundParent      = false;

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
        return;
    }
    Parents[childID - 1U] = 0;
    for (size_t i = parentIndex; i < Order.size(); ++i) {
        const auto& entityID = Order[i];
        if (entityID == childID) {
            Order[i] = 0;

            //now move all children of parent to be next to parent
            for (size_t j = i + 1U; j < Order.size(); ++j) {
                const auto& entityIDCaseOne = Order[j];
                if (Order[j] == 0) {
                    break;
                }
                if (Parents[entityIDCaseOne - 1U] == parentID) {
                    std::swap(Order[j - 1U], Order[j]);
                }else if (Parents[entityIDCaseOne - 1U] == childID && Order[j - 1U] == 0) {
                    Order[j - 1U] = childID;
                }else if (Parents[entityIDCaseOne - 1U] == 0) {
                    break;
                }
            }
            //cleanup / edge cases
            if (Order[i + 1U] == 0) {
                Order.erase(Order.begin() + i);
                --OrderHead;
            }
            if (parentIndex > 0 && Order[parentIndex + 1U] == 0) {
                Order[parentIndex] = 0;
                Order.erase(Order.begin() + parentIndex);
                --OrderHead;
            }
            break;
        }
    }
}
const std::uint32_t Engine::priv::ComponentBody_System::ParentChildVector::size() const {
    return OrderHead;
}
const size_t Engine::priv::ComponentBody_System::ParentChildVector::capacity() const {
    return Order.capacity();
}


Engine::priv::ComponentBody_System::ComponentBody_System(const Engine::priv::ECSSystemCI& systemCI, Engine::priv::ECS<Entity>& ecs) : Engine::priv::ECSSystem<Entity, ComponentBody>(systemCI, ecs) {

}
Engine::priv::ComponentBody_System::~ComponentBody_System() {

}





#pragma endregion
#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/ModelInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Camera.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <BulletCollision/Gimpact/btGImpactShape.h>

#include <iostream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region PhysicsData

ComponentBody::PhysicsData::PhysicsData(){ 
    //constructor
    mass             = 0;
    group            = CollisionFilter::DefaultFilter;
    mask             = CollisionFilter::AllFilter;
    bullet_rigidBody = nullptr;
    collision        = nullptr;
}
ComponentBody::PhysicsData::PhysicsData(ComponentBody::PhysicsData&& p_Other) noexcept {
    //move constructor
    using std::swap;
    swap(mass, p_Other.mass);
    swap(bullet_motionState, p_Other.bullet_motionState);
    swap(group, p_Other.group);
    swap(mask, p_Other.mask);

    if (p_Other.collision)
        swap(collision, p_Other.collision);
    else
        collision = nullptr;
    if (p_Other.bullet_rigidBody)
        swap(bullet_rigidBody, p_Other.bullet_rigidBody);
    else
        bullet_rigidBody = nullptr;
}
ComponentBody::PhysicsData& ComponentBody::PhysicsData::operator=(ComponentBody::PhysicsData&& p_Other) noexcept {
    //move assignment
    using std::swap;
    swap(mass, p_Other.mass);
    swap(bullet_motionState, p_Other.bullet_motionState);
    swap(group, p_Other.group);
    swap(mask, p_Other.mask);
    if (p_Other.collision) 
        swap(collision, p_Other.collision);
    else                   
        collision = nullptr;
    if (p_Other.bullet_rigidBody) 
        swap(bullet_rigidBody, p_Other.bullet_rigidBody);
    else                   
        bullet_rigidBody = nullptr;
    return *this;
}
ComponentBody::PhysicsData::~PhysicsData() {
    //destructor
    //SAFE_DELETE(collision);
    Physics::removeRigidBody(bullet_rigidBody);
    SAFE_DELETE(bullet_rigidBody);
}

#pragma endregion

#pragma region NormalData

ComponentBody::NormalData::NormalData(){
    //constructor
    scale       = glm_vec3(static_cast<decimal>(1.0));
    position    = glm_vec3(static_cast<decimal>(0.0));
    rotation    = glm_quat(
        static_cast<decimal>(1.0), 
        static_cast<decimal>(0.0),
        static_cast<decimal>(0.0),
        static_cast<decimal>(0.0)
    );
    modelMatrix = glm_mat4(static_cast<decimal>(1.0));
}
ComponentBody::NormalData::NormalData(ComponentBody::NormalData&& other) noexcept {
    //move constructor
    using std::swap;
    swap(position, other.position);
    swap(rotation, other.rotation);
    swap(scale, other.scale);
    swap(modelMatrix, other.modelMatrix);
}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(ComponentBody::NormalData&& p_Other) noexcept {
    //move assignment
    using std::swap;
    swap(position, p_Other.position);
    swap(rotation, p_Other.rotation);
    swap(scale, p_Other.scale);
    swap(modelMatrix, p_Other.modelMatrix);
    return *this;
}
ComponentBody::NormalData::~NormalData() {
    //destructor
}


#pragma endregion

#pragma region Component

ComponentBody::ComponentBody(const Entity& p_Entity) : ComponentBaseClass(p_Entity) {
    m_GoalSpeed               = static_cast<decimal>(1.0);
    m_Goal = m_GoalVelocity   = glm_vec3(static_cast<decimal>(0.0));
    m_Physics                 = false;
    m_UserPointer             = nullptr;
    m_UserPointer1            = nullptr;
    m_UserPointer2            = nullptr;
    data.p                    = nullptr;
    data.n                    = new NormalData();
    setCollisionFunctor(ComponentBody_EmptyCollisionFunctor());
    auto& normalData          = *data.n;
    normalData.position       = glm_vec3(static_cast<decimal>(0.0));
    normalData.scale          = glm_vec3(static_cast<decimal>(1.0));
    normalData.rotation       = glm_quat(
        static_cast<decimal>(1.0),
        static_cast<decimal>(0.0),
        static_cast<decimal>(0.0),
        static_cast<decimal>(0.0)
    );
    normalData.modelMatrix    = glm_mat4(static_cast<decimal>(1.0));
    Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
}
ComponentBody::ComponentBody(const Entity& p_Entity, const CollisionType::Type p_CollisionType) : ComponentBaseClass(p_Entity) {
    m_GoalSpeed             = static_cast<decimal>(1.0);
    m_Goal = m_GoalVelocity = glm_vec3(static_cast<decimal>(0.0));
    m_Physics               = true;
    m_UserPointer           = nullptr;
    m_UserPointer1          = nullptr;
    m_UserPointer2          = nullptr;
    data.n                  = nullptr;
    data.p                  = new PhysicsData();
    auto& physicsData       = *data.p;
    m_Forward               = glm_vec3(static_cast<decimal>(0.0), static_cast<decimal>(0.0), static_cast<decimal>(-1.0));
	m_Right                 = glm_vec3(static_cast<decimal>(1.0), static_cast<decimal>(0.0), static_cast<decimal>(0.0));
	m_Up                    = glm_vec3(static_cast<decimal>(0.0), static_cast<decimal>(1.0), static_cast<decimal>(0.0));

    setCollisionFunctor(ComponentBody_EmptyCollisionFunctor());

    physicsData.bullet_motionState = btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    setCollision(p_CollisionType, 1.0f);
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
ComponentBody::ComponentBody(ComponentBody&& p_Other) noexcept {
    //move constructor
    using std::swap;
    swap(m_Physics,p_Other.m_Physics);
    swap(m_Forward, p_Other.m_Forward);
    swap(m_Right, p_Other.m_Right);
    swap(m_Up, p_Other.m_Up);
    swap(owner.data, p_Other.owner.data);
    swap(m_CollisionFunctor, p_Other.m_CollisionFunctor);
    swap(m_UserPointer, p_Other.m_UserPointer);
    swap(m_UserPointer1, p_Other.m_UserPointer1);
    swap(m_UserPointer2, p_Other.m_UserPointer2);
    swap(m_Goal, p_Other.m_Goal);
    swap(m_GoalSpeed, p_Other.m_GoalSpeed);
    if (p_Other.m_Physics) {
        swap(data.p, p_Other.data.p);
		p_Other.data.p = nullptr;
    }else{
        swap(data.n, p_Other.data.n);
		p_Other.data.n = nullptr;
    }
    setInternalPhysicsUserPointer(this);
}
ComponentBody& ComponentBody::operator=(ComponentBody&& p_Other) noexcept {
    //move assignment
    using std::swap;
    swap(m_Physics,p_Other.m_Physics);
    swap(m_Forward, p_Other.m_Forward);
    swap(m_Right, p_Other.m_Right);
    swap(m_Up, p_Other.m_Up);
    swap(owner.data, p_Other.owner.data);
    swap(m_CollisionFunctor, p_Other.m_CollisionFunctor);
    swap(m_UserPointer, p_Other.m_UserPointer);
    swap(m_UserPointer1, p_Other.m_UserPointer1);
    swap(m_UserPointer2, p_Other.m_UserPointer2);
    swap(m_Goal, p_Other.m_Goal);
    swap(m_GoalSpeed, p_Other.m_GoalSpeed);
    if (p_Other.m_Physics) {
        swap(data.p, p_Other.data.p);
    }else{
        swap(data.n, p_Other.data.n);
    }
    setInternalPhysicsUserPointer(this);
    return *this;
}
void ComponentBody::rebuildRigidBody(const bool addBodyToPhysicsWorld) {
    if (m_Physics) {
        auto& physics = *data.p;
        if (physics.bullet_rigidBody) {
            Physics::removeRigidBody(physics.bullet_rigidBody);
            SAFE_DELETE(physics.bullet_rigidBody);
        }
        auto& inertia = physics.collision->getBtInertia();
        auto& shape = *physics.collision->getBtShape();
        btRigidBody::btRigidBodyConstructionInfo CI(physics.mass, &physics.bullet_motionState, &shape, inertia);
        physics.bullet_rigidBody = new btRigidBody(CI);
        auto& rigidBody = *physics.bullet_rigidBody;
        rigidBody.setSleepingThresholds(0.015f, 0.015f);
        rigidBody.setFriction(0.3f);
        setDamping(static_cast<decimal>(0.1), static_cast<decimal>(0.4));
        rigidBody.setMassProps(static_cast<btScalar>(physics.mass), inertia);
        rigidBody.updateInertiaTensor();
        setInternalPhysicsUserPointer(this);
        if(addBodyToPhysicsWorld)
            Physics::addRigidBody(physics.bullet_rigidBody, physics.group, physics.mask);
    }
}
//kinda ugly
void ComponentBody::setInternalPhysicsUserPointer(void* userPtr) {
    if (m_Physics) {
        auto* rigid = data.p->bullet_rigidBody;
        if (rigid) {
            rigid->setUserPointer(userPtr);
            auto* shape = rigid->getCollisionShape();
            if (shape) {
                shape->setUserPointer(userPtr);
                if (data.p->collision && data.p->collision->getType() == CollisionType::Compound) {
                    auto* compound = dynamic_cast<btCompoundShape*>(shape);
                    if (compound) {
                        const auto numChildren = compound->getNumChildShapes();
                        compound->setUserPointer(userPtr); //do we need this?
                        if (numChildren > 0) {
                            for (int i = 0; i < numChildren; ++i) {
                                btCollisionShape* child_shape = compound->getChildShape(i);
                                child_shape->setUserPointer(userPtr);
                            }
                        }
                    }
                }
            }
        }
    }
}
const decimal& ComponentBody::getGoalSpeed() const {
    return m_GoalSpeed;
}
const glm_vec3& ComponentBody::getGoal() const {
    return m_Goal;
}
void ComponentBody::setGoal(const glm_vec3& _goal, const decimal& speed) {
    m_Goal      = _goal;
    m_GoalSpeed = speed;
}
void ComponentBody::setGoal(const decimal& x, const decimal& y, const decimal& z, const decimal& speed) {
    m_Goal.x    = x; m_Goal.y = y; m_Goal.z = z;
    m_GoalSpeed = speed;
}
const bool& ComponentBody::hasPhysics() const {
    return m_Physics;
}
void ComponentBody::setUserPointer(void* userPtr) {
    m_UserPointer = userPtr;
}
void ComponentBody::setUserPointer1(void* userPtr) {
    m_UserPointer1 = userPtr;
}
void ComponentBody::setUserPointer2(void* userPtr) {
    m_UserPointer2 = userPtr;
}
void* ComponentBody::getUserPointer() {
    return m_UserPointer;
}
void* ComponentBody::getUserPointer1() {
    return m_UserPointer1;
}
void* ComponentBody::getUserPointer2() {
    return m_UserPointer2;
}
void ComponentBody::collisionResponse(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) {
    if (m_CollisionFunctor.vtable && !m_CollisionFunctor.empty()) { //hacky, but needed for some reason...
        m_CollisionFunctor(std::ref(owner), std::ref(ownerHit), std::ref(other), std::ref(otherHit), std::ref(normal));
    }
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

const decimal ComponentBody::getDistance(const Entity& p_Other) {
    const auto& other_position = const_cast<Entity&>(p_Other).getComponent<ComponentBody>()->position();
    return glm::distance(position(), other_position);
}
const unsigned long long ComponentBody::getDistanceLL(const Entity& p_Other) {
    const auto& other_position = const_cast<Entity&>(p_Other).getComponent<ComponentBody>()->position();
    return static_cast<unsigned long long>(glm::distance(position(), other_position));
}
void ComponentBody::alignTo(const glm_vec3& p_Direction, const decimal p_Speed) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        //recheck this
        btTransform tr;
        rigidBody.getMotionState()->getWorldTransform(tr);
        //Math::alignTo(Math::btToGLMQuat(tr.getRotation()),direction,speed);
        Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(normalData.rotation, p_Direction, p_Speed);
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
Collision* ComponentBody::getCollision() {
    if (m_Physics) {
        return data.p->collision;
    }
    return nullptr;
}
void ComponentBody::setCollision(const CollisionType::Type p_CollisionType, const float p_Mass) {
    auto& physicsData = *data.p;
    if (!physicsData.collision) { //TODO: clean this up, its hacky and evil. its being used on the ComponentBody_EntityAddedToSceneFunction
        auto* modelComponent = owner.getComponent<ComponentModel>();
        if (modelComponent) {
            if (p_CollisionType == CollisionType::Compound) {
                physicsData.collision = new Collision(this, *modelComponent, p_Mass);
            }else{
                physicsData.collision = new Collision(p_CollisionType, modelComponent->getModel().mesh(), p_Mass);
            }
        }else{
            physicsData.collision = new Collision(p_CollisionType, nullptr, p_Mass);
        }
    }
    physicsData.mass = p_Mass;
    Collision& collision = *physicsData.collision;
    auto& shape = *collision.getBtShape();
    collision.setMass(physicsData.mass);
    if (physicsData.bullet_rigidBody) {
        auto& bt_rigidBody = *physicsData.bullet_rigidBody;
        bt_rigidBody.setCollisionShape(&shape);
        bt_rigidBody.setMassProps(static_cast<btScalar>(physicsData.mass), collision.getBtInertia());
        bt_rigidBody.updateInertiaTensor();
    }
    setInternalPhysicsUserPointer(this);
}
//double check this...
void ComponentBody::setCollision(Collision* p_Collision) {
    auto& physicsData = *data.p;
    if (physicsData.collision) {
        Physics::removeRigidBody(physicsData.bullet_rigidBody);
        SAFE_DELETE(physicsData.collision);
    }
    physicsData.collision = p_Collision;
    Collision& collision = *physicsData.collision;
    auto& shape = *collision.getBtShape();
    if (physicsData.bullet_rigidBody) {
        auto& bt_rigidBody = *physicsData.bullet_rigidBody;
        bt_rigidBody.setCollisionShape(&shape);
        auto& inertia = collision.getBtInertia();
        bt_rigidBody.setMassProps(static_cast<btScalar>(physicsData.mass), inertia);
        bt_rigidBody.updateInertiaTensor();
        Physics::addRigidBody(&bt_rigidBody, physicsData.group, physicsData.mask);
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
        auto& bt_rigidBody = *data.p->bullet_rigidBody;
        bt_rigidBody.activate();
        btVector3 v(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        Math::translate(bt_rigidBody, v, p_Local);
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
    auto* models = owner.getComponent<ComponentModel>();
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
        auto& physicsData = *data.p;
        btTransform tr;
        tr.setOrigin(btVector3(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z)));
        tr.setRotation(physicsData.bullet_rigidBody->getOrientation());
        Collision& collision = *physicsData.collision;
        if (collision.getType() == CollisionType::TriangleShapeStatic) {
            Physics::removeRigidBody(physicsData.bullet_rigidBody);
        }
        physicsData.bullet_motionState.setWorldTransform(tr);
        physicsData.bullet_rigidBody->setMotionState(&physicsData.bullet_motionState); //is this needed?
        physicsData.bullet_rigidBody->setWorldTransform(tr);
        physicsData.bullet_rigidBody->setCenterOfMassTransform(tr);
        if (collision.getType() == CollisionType::TriangleShapeStatic) {
            Physics::addRigidBody(physicsData.bullet_rigidBody, physicsData.group, physicsData.mask);
        }
    }else{
        auto& normalData = *data.n;
        auto& position_ = normalData.position;
        auto& modelMatrix_ = normalData.modelMatrix;
		position_.x = p_X;
		position_.y = p_Y;
		position_.z = p_Z;
		modelMatrix_[3][0] = p_X;
		modelMatrix_[3][1] = p_Y;
		modelMatrix_[3][2] = p_Z;
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
void ComponentBody::setRotation(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const decimal& p_W) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        btQuaternion quat(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z), static_cast<btScalar>(p_W));
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
        glm_quat newRotation(p_W, p_X, p_Y, p_Z);
        newRotation = glm::normalize(newRotation);
        auto& rotation_ = normalData.rotation;
		rotation_ = newRotation;
        Math::recalculateForwardRightUp(rotation_, m_Forward, m_Right, m_Up);
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
        const auto& newScale = btVector3(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
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
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        epriv::ComponentModel_Functions::CalculateRadius(*models);
    }
}
const glm_vec3 ComponentBody::position() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        physicsData.bullet_rigidBody->getMotionState()->getWorldTransform(tr);
        return Math::btVectorToGLM(tr.getOrigin());
    }
    const auto& modelMatrix_ = data.n->modelMatrix;
    return glm_vec3(modelMatrix_[3][0], modelMatrix_[3][1], modelMatrix_[3][2]);
}
const glm::vec3 ComponentBody::position_render() const { //theres prob a better way to do this
    if (m_Physics) {
        auto tr = data.p->bullet_rigidBody->getWorldTransform();
        return Math::btVectorToGLM(tr.getOrigin());
    }
    const auto& modelMatrix_ = data.n->modelMatrix;
    return glm::vec3(modelMatrix_[3][0], modelMatrix_[3][1], modelMatrix_[3][2]);
}
glm::vec3 ComponentBody::getScreenCoordinates(const bool p_ClampToEdge) {
	return Math::getScreenCoordinates(position(), *owner.scene().getActiveCamera(), p_ClampToEdge);
}
ScreenBoxCoordinates ComponentBody::getScreenBoxCoordinates(const float p_MinOffset) {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = position();
    auto radius             = 0.0001f;
    auto* model             = owner.getComponent<ComponentModel>();
    auto& camera            = *owner.scene().getActiveCamera();
    const auto& center2DRes = Math::getScreenCoordinates(worldPos, camera, false);
    auto center2D           = glm::vec2(center2DRes.x, center2DRes.y);
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
    const auto& camvectest       = cam.up();   
    const auto  testRes          = Math::getScreenCoordinates(worldPos + (camvectest * static_cast<decimal>(radius)), camera, false);
    const auto test              = glm::vec2(testRes.x, testRes.y);
    const auto radius2D          = glm::max(p_MinOffset, glm::distance(test, center2D));
    const auto& yPlus            = center2D.y + radius2D;
    const auto& yNeg             = center2D.y - radius2D;
    const auto& xPlus            = center2D.x + radius2D;
    const auto& xNeg             = center2D.x - radius2D;
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
    return glm_vec3(static_cast<decimal>(0.0));
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
    return data.n->modelMatrix;
}
const glm::mat4 ComponentBody::modelMatrixRendering() const {
    glm::mat4 ret;
    ret = static_cast<glm::mat4>(modelMatrix());
    return ret;
}
const btRigidBody& ComponentBody::getBtBody() const {
	return *data.p->bullet_rigidBody;
}
void ComponentBody::setDamping(const decimal& p_LinearFactor, const decimal& p_AngularFactor) {
	data.p->bullet_rigidBody->setDamping(static_cast<btScalar>(p_LinearFactor), static_cast<btScalar>(p_AngularFactor));
}
void ComponentBody::setCollisionGroup(const short& group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != group) {
            Physics::removeRigidBody(phyData.bullet_rigidBody);
            phyData.group = group;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::setCollisionMask(const short& mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != mask) {
            Physics::removeRigidBody(phyData.bullet_rigidBody);
            phyData.mask = mask;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::setCollisionGroup(const CollisionFilter::Filter& group) {
    ComponentBody::setCollisionGroup(static_cast<short>(group));
}
void ComponentBody::setCollisionMask(const CollisionFilter::Filter& mask) {
    ComponentBody::setCollisionMask(static_cast<short>(mask));
}
void ComponentBody::addCollisionGroup(const short& group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != (phyData.group | group)) {
            Physics::removeRigidBody(phyData.bullet_rigidBody);
            phyData.group = phyData.group | group;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::addCollisionMask(const short& mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != (phyData.mask | mask)) {
            Physics::removeRigidBody(phyData.bullet_rigidBody);
            phyData.mask = phyData.mask | mask;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::addCollisionGroup(const CollisionFilter::Filter& group) {
    ComponentBody::addCollisionGroup(static_cast<short>(group));
}
void ComponentBody::addCollisionMask(const CollisionFilter::Filter& mask) {
    ComponentBody::addCollisionMask(static_cast<short>(mask));
}
void ComponentBody::setCollisionFlag(const short& flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != flag) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(flag);
            Physics::addRigidBody(&rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::setCollisionFlag(const CollisionFlag::Flag& flag) {
    ComponentBody::setCollisionFlag(static_cast<short>(flag));
}
void ComponentBody::addCollisionFlag(const short& flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != (currFlags | flag)) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(currFlags | flag);
            Physics::addRigidBody(&rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::addCollisionFlag(const CollisionFlag::Flag& flag) {
    ComponentBody::addCollisionFlag(static_cast<short>(flag));
}
void ComponentBody::removeCollisionGroup(const short& group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != (phyData.group & ~group)) {
            Physics::removeRigidBody(phyData.bullet_rigidBody);
            phyData.group = phyData.group & ~group;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::removeCollisionMask(const short& mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != (phyData.mask & ~mask)) {
            Physics::removeRigidBody(phyData.bullet_rigidBody);
            phyData.mask = phyData.mask & ~mask;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::removeCollisionFlag(const short& flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.bullet_rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != (currFlags & ~flag)) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(currFlags & ~flag);
            Physics::addRigidBody(&rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::removeCollisionGroup(const CollisionFilter::Filter& group) {
    ComponentBody::removeCollisionGroup(static_cast<short>(group));
}
void ComponentBody::removeCollisionMask(const CollisionFilter::Filter& mask) {
    ComponentBody::removeCollisionMask(static_cast<short>(mask));
}
void ComponentBody::removeCollisionFlag(const CollisionFlag::Flag& flag) {
    ComponentBody::removeCollisionFlag(static_cast<short>(flag));
}


//TODO: reconsider how this works
void ComponentBody::setDynamic(const bool p_Dynamic) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        auto& rigidBody = *physicsData.bullet_rigidBody;
        if (p_Dynamic) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            Physics::addRigidBody(&rigidBody, physicsData.group, physicsData.mask);
            rigidBody.activate();
        }else{
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            Physics::addRigidBody(&rigidBody, physicsData.group, physicsData.mask);
            rigidBody.activate();
        }
    }
}
void ComponentBody::setLinearVelocity(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        Math::translate(rigidBody, v, p_Local);
        rigidBody.setLinearVelocity(v);
    }
}
void ComponentBody::setLinearVelocity(const glm_vec3& p_Velocity, const bool p_Local) {
	ComponentBody::setLinearVelocity(p_Velocity.x, p_Velocity.y, p_Velocity.z, p_Local);
}
void ComponentBody::setAngularVelocity(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
		auto& rigidBody = *data.p->bullet_rigidBody;
		rigidBody.activate();
		btVector3 v(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
		Math::translate(rigidBody, v, p_Local);
		rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setAngularVelocity(const glm_vec3& p_Velocity, const bool p_Local) {
	ComponentBody::setAngularVelocity(p_Velocity.x, p_Velocity.y, p_Velocity.z, p_Local);
}
void ComponentBody::applyForce(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyCentralForce(v);
    }
}
void ComponentBody::applyForce(const glm_vec3& p_Force, const glm_vec3& p_Origin, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(p_Force.x), static_cast<btScalar>(p_Force.y), static_cast<btScalar>(p_Force.z));
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyForce(v, btVector3(static_cast<btScalar>(p_Origin.x), static_cast<btScalar>(p_Origin.y), static_cast<btScalar>(p_Origin.z)));
    }
}
void ComponentBody::applyImpulse(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(const glm_vec3& p_Impulse, const glm_vec3& p_Origin, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 v(static_cast<btScalar>(p_Impulse.x), static_cast<btScalar>(p_Impulse.y), static_cast<btScalar>(p_Impulse.z));
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyImpulse(v, btVector3(static_cast<btScalar>(p_Origin.x), static_cast<btScalar>(p_Origin.y), static_cast<btScalar>(p_Origin.z)));
    }
}
void ComponentBody::applyTorque(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 t(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        if (p_Local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorque(t);
    }
}
void ComponentBody::applyTorque(const glm_vec3& p_Torque, const bool p_Local) {
	ComponentBody::applyTorque(p_Torque.x, p_Torque.y, p_Torque.z, p_Local);
}
void ComponentBody::applyTorqueImpulse(const decimal& p_X, const decimal& p_Y, const decimal& p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->bullet_rigidBody;
        rigidBody.activate();
        btVector3 t(static_cast<btScalar>(p_X), static_cast<btScalar>(p_Y), static_cast<btScalar>(p_Z));
        if (p_Local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorqueImpulse(t);
    }
}
void ComponentBody::applyTorqueImpulse(const glm_vec3& p_TorqueImpulse, const bool p_Local) {
	ComponentBody::applyTorqueImpulse(p_TorqueImpulse.x, p_TorqueImpulse.y, p_TorqueImpulse.z, p_Local);
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
void ComponentBody::setMass(const float p_Mass) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        physicsData.mass = p_Mass;
        Collision& collision = *physicsData.collision;
        if (collision.getBtShape()) {
            /*
            auto* compound = dynamic_cast<btCompoundShape*>(collision.getBtShape());
            if (compound) {
                btVector3& inertia = const_cast<btVector3&>(collision.getBtInertia());
                const auto numChildren = compound->getNumChildShapes();
                if (numChildren > 0) {
                    for (int i = 0; i < numChildren; ++i) {
                        auto* child_shape = compound->getChildShape(i);
                        if (child_shape) {
                            child_shape->calculateLocalInertia(physicsData.mass, inertia);
                        }
                    }
                }
            }
            */
            collision.setMass(physicsData.mass);
            if (physicsData.bullet_rigidBody) {
                physicsData.bullet_rigidBody->setMassProps(static_cast<btScalar>(physicsData.mass), collision.getBtInertia());
            }
        }
    }
}

#pragma endregion

#pragma region System

struct epriv::ComponentBody_UpdateFunction final {
    static void _defaultUpdate(const vector<uint>& p_Vector, vector<ComponentBody>& p_Components, const float& dt) {
        for (uint j = 0; j < p_Vector.size(); ++j) {
            ComponentBody& b = p_Components[p_Vector[j]];
            const auto pos = b.position();
            //TODO: goals need to be properly tested and fixed
            const auto zero = glm_vec3(0.0);
            if (b.m_Goal != zero){
                if (glm::distance2(b.m_Goal, pos) > static_cast<decimal>(1.5)) {
                    const auto vecTo = pos - b.m_Goal;
                    const auto vecToNorm = glm::normalize(vecTo);
                    if (b.m_GoalVelocity == zero) {
                        //calc goal velocity
                        const auto len = glm::length(vecTo);
                        const auto velPerSec = len * dt;
                        const auto velSpeed = velPerSec * b.m_GoalSpeed;
                        b.m_GoalVelocity = vecToNorm * velSpeed;
                    }
                    const auto goal = pos + b.m_GoalVelocity;
                    if (glm::length2(goal) > glm::length2(vecTo)) {
                        b.setPosition(goal);
                    }else{
                        b.setPosition(b.m_Goal);
                    }
                }else{
                    b.m_Goal = b.m_GoalVelocity = glm_vec3(static_cast<decimal>(0.0));
                }
            }
            if (b.m_Physics) {
                auto& rigidBody = *b.data.p->bullet_rigidBody;
                Engine::Math::recalculateForwardRightUp(rigidBody, b.m_Forward, b.m_Right, b.m_Up);
            }else{
                auto& n = *b.data.n;
                //TODO: implement parent->child relations
                //n.modelMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale) * n.modelMatrix;
                n.modelMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);
                //Engine::Math::recalculateForwardRightUp(n.rotation, b._forward, b._right, b._up); //double check if this is needed
            }
        }
    }
    void operator()(void* p_ComponentPool, const double& dt, Scene& p_Scene) const {
        auto& pool = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool);
        auto& components = pool.pool();
        auto split = epriv::threading::splitVectorIndices(components);
        const auto fdt = static_cast<float>(dt);
        for (auto& vec : split) {
            epriv::threading::addJobRef(_defaultUpdate, vec, components, fdt);
        }
        epriv::threading::waitForAll();
    }
};
struct epriv::ComponentBody_ComponentAddedToEntityFunction final {void operator()(void* p_Component, Entity& p_Entity) const {
}};
struct epriv::ComponentBody_EntityAddedToSceneFunction final {void operator()(void* p_ComponentPool,Entity& p_Entity, Scene& p_Scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool);
    auto* _component = pool.getComponent(p_Entity);
    if (_component) {
        auto& component = *_component;
        if (component.m_Physics) {
            auto& physicsData = *component.data.p;
            component.setCollision(static_cast<CollisionType::Type>(physicsData.collision->getType()), physicsData.mass);
            auto currentScene = Resources::getCurrentScene();
            if (currentScene && currentScene == &p_Scene) {
                auto& phyData = *component.data.p;
                Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
            }
        }
    }
}};
struct epriv::ComponentBody_SceneEnteredFunction final {void operator()(void* p_ComponentPool,Scene& p_Scene) const {
	auto& pool = (*static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool)).pool();
    for (auto& component : pool) { 
        if (component.m_Physics) {
            auto& phyData = *component.data.p;
            Physics::addRigidBody(phyData.bullet_rigidBody, phyData.group, phyData.mask);
        } 
    }
}};
struct epriv::ComponentBody_SceneLeftFunction final {void operator()(void* p_ComponentPool, Scene& p_Scene) const {
	auto& pool = (*static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool)).pool();
    for (auto& component : pool) { 
        if (component.m_Physics) {
            auto& phyData = *component.data.p;
            Physics::removeRigidBody(phyData.bullet_rigidBody);
        } 
    }
}};
    
ComponentBody_System::ComponentBody_System() {
    setUpdateFunction(ComponentBody_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentBody_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentBody_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentBody_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentBody_SceneLeftFunction());
}

#pragma endregion
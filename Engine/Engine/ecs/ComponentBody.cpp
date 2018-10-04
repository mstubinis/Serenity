#include "ComponentBody.h"
#include "ComponentModel.h"
#include "../Engine_Math.h"
#include "../MeshInstance.h"
#include "../Mesh.h"

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;

ComponentBody::ComponentBody(Entity& _e) : ComponentBaseClass(_e) {
    data.n = new NormalData();
    _physics = false;
    auto& normalData = *data.n;
    normalData.position = new glm::vec3(0.0f);
    normalData.scale = new glm::vec3(1.0f);
    normalData.rotation = new glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    normalData.modelMatrix = new glm::mat4(1.0f);
    Math::recalculateForwardRightUp(*normalData.rotation, _forward, _right, _up);
}
ComponentBody::ComponentBody(Entity& _e,CollisionType::Type _collisionType) : ComponentBaseClass(_e) {
    data.p = new PhysicsData();
    _physics = true;
    auto& physicsData = *data.p;
    _forward = glm::vec3(0, 0, -1);  _right = glm::vec3(1, 0, 0);  _up = glm::vec3(0, 1, 0);

    physicsData.motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    float _mass = 1.0f;
    physicsData.mass = _mass;

    setCollision(_collisionType, _mass);

    setMass(_mass);
    btDefaultMotionState* _motionState = physicsData.motionState;
    btCollisionShape* _shape = physicsData.collision->getShape();
    const btVector3& _inertia = physicsData.collision->getInertia();

    btRigidBody::btRigidBodyConstructionInfo CI(_mass, _motionState, _shape, _inertia);
    physicsData.rigidBody = new btRigidBody(CI);
    auto& rigidBody = *physicsData.rigidBody;
    rigidBody.setSleepingThresholds(0.015f, 0.015f);
    rigidBody.setFriction(0.3f);
    rigidBody.setDamping(0.1f, 0.4f);//air friction 
    rigidBody.setMassProps(_mass, _inertia);
    rigidBody.updateInertiaTensor();
    rigidBody.setUserPointer(this);
}
ComponentBody::~ComponentBody() {
    if (_physics) {
        auto& physicsData = *data.p;
        Physics::removeRigidBody(physicsData.rigidBody);
        SAFE_DELETE(physicsData.rigidBody);
        SAFE_DELETE(physicsData.collision);
        SAFE_DELETE(physicsData.motionState);
        SAFE_DELETE(data.p);
    }else{
        auto& normalData = *data.n;
        SAFE_DELETE(normalData.position);
        SAFE_DELETE(normalData.scale);
        SAFE_DELETE(normalData.rotation);
        SAFE_DELETE(normalData.modelMatrix);
        SAFE_DELETE(data.n);
    }
}
void ComponentBody::alignTo(glm::vec3 direction, float speed) {
    if (_physics) {
        auto& physicsData = *data.p;
        //recheck this
        btTransform tr;
        physicsData.rigidBody->getMotionState()->getWorldTransform(tr);
        //Math::alignTo(Math::btToGLMQuat(tr.getRotation()),direction,speed);
        Math::recalculateForwardRightUp(physicsData.rigidBody, _forward, _right, _up);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(*normalData.rotation, direction, speed);
        Math::recalculateForwardRightUp(*normalData.rotation, _forward, _right, _up);
    }
}
void ComponentBody::setCollision(CollisionType::Type _type, float _mass) {
    auto& physicsData = *data.p;
    SAFE_DELETE(physicsData.collision);
    ComponentModel* modelComponent = owner.getComponent<ComponentModel>();
    if (modelComponent) {
        if (_type == CollisionType::Compound) {
            physicsData.collision = new Collision(*modelComponent, _mass);
        }else{
            physicsData.collision = new Collision(_type, modelComponent->getModel()->mesh(), _mass);
        }
    }else{
        physicsData.collision = new Collision(_type, nullptr, _mass);
    }
    physicsData.collision->getShape()->setUserPointer(this);
    if (physicsData.rigidBody) {
        Physics::removeRigidBody(physicsData.rigidBody);
        physicsData.rigidBody->setCollisionShape(physicsData.collision->getShape());
        physicsData.rigidBody->setMassProps(physicsData.mass, physicsData.collision->getInertia());
        physicsData.rigidBody->updateInertiaTensor();
        Physics::addRigidBody(physicsData.rigidBody);
    }
}
void ComponentBody::translate(glm::vec3 translation, bool local) { ComponentBody::translate(translation.x, translation.y, translation.z, local); }
void ComponentBody::translate(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        setPosition(position() + Engine::Math::btVectorToGLM(v));
    }else{
        auto& normalData = *data.n;
        glm::vec3& _position = *normalData.position;
        _position.x += x; _position.y += y; _position.z += z;
        glm::vec3 offset(x, y, z);
        if (local) {
            offset = *(normalData.rotation) * offset;
        }
        setPosition(_position + offset);
    }
}
void ComponentBody::rotate(glm::vec3 rotation, bool local) { ComponentBody::rotate(rotation.x, rotation.y, rotation.z, local); }
void ComponentBody::rotate(float pitch, float yaw, float roll, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        btQuaternion quat = physicsData.rigidBody->getWorldTransform().getRotation().normalize();
        glm::quat glmquat(quat.w(), quat.x(), quat.y(), quat.z());

        if (abs(pitch) >= 0.001f) glmquat = glmquat * (glm::angleAxis(-pitch, glm::vec3(1, 0, 0)));
        if (abs(yaw) >= 0.001f)   glmquat = glmquat * (glm::angleAxis(-yaw, glm::vec3(0, 1, 0)));
        if (abs(roll) >= 0.001f)  glmquat = glmquat * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));

        quat = btQuaternion(glmquat.x, glmquat.y, glmquat.z, glmquat.w);
        physicsData.rigidBody->getWorldTransform().setRotation(quat);
    }else{
        auto& normalData = *data.n;
        glm::quat& _rotation = *normalData.rotation;
        if (abs(pitch) >= 0.001f) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1, 0, 0)));
        if (abs(yaw) >= 0.001f)   _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0, 1, 0)));
        if (abs(roll) >= 0.001f)  _rotation = _rotation * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));
        Math::recalculateForwardRightUp(_rotation, _forward, _right, _up);
    }
}
void ComponentBody::scale(glm::vec3 amount) { ComponentBody::scale(amount.x, amount.y, amount.z); }
void ComponentBody::scale(float x, float y, float z) {
    if (_physics) {
        auto& physicsData = *data.p;
        if (physicsData.collision) {
            if (physicsData.collision->getType() == CollisionType::Compound) {
                btCompoundShape* cast = ((btCompoundShape*)(physicsData.collision->getShape()));
                for (int i = 0; i < cast->getNumChildShapes(); ++i) {
                    btCollisionShape* shape = cast->getChildShape(i);
                    btUniformScalingShape* convexHull = dynamic_cast<btUniformScalingShape*>(shape);
                    if (convexHull) {
                        convexHull->setLocalScaling(convexHull->getLocalScaling() + btVector3(x, y, z));
                        continue;
                    }
                    btScaledBvhTriangleMeshShape* triHull = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                    if (triHull) {
                        triHull->setLocalScaling(triHull->getLocalScaling() + btVector3(x, y, z));
                    }
                }
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& _scale = *normalData.scale;
        _scale.x += x; _scale.y += y; _scale.z += z;
    }
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        //epriv::ComponentInternalFunctionality::CalculateRadius(*models);
    }
}
void ComponentBody::setPosition(glm::vec3 newPosition) { ComponentBody::setPosition(newPosition.x, newPosition.y, newPosition.z); }
void ComponentBody::setPosition(float x, float y, float z) {
    if (_physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        tr.setOrigin(btVector3(x, y, z));
        tr.setRotation(physicsData.rigidBody->getOrientation());
        if (physicsData.collision->getType() == CollisionType::TriangleShapeStatic) {
            Physics::removeRigidBody(physicsData.rigidBody);
            SAFE_DELETE(physicsData.rigidBody);
        }
        physicsData.motionState->setWorldTransform(tr);
        if (physicsData.collision->getType() == CollisionType::TriangleShapeStatic) {
            btRigidBody::btRigidBodyConstructionInfo ci(physicsData.mass, physicsData.motionState, physicsData.collision->getShape(), physicsData.collision->getInertia());
            physicsData.rigidBody = new btRigidBody(ci);
            physicsData.rigidBody->setUserPointer(this);
            Physics::addRigidBody(physicsData.rigidBody);
        }
        physicsData.rigidBody->setMotionState(physicsData.motionState); //is this needed?
        physicsData.rigidBody->setWorldTransform(tr);
        physicsData.rigidBody->setCenterOfMassTransform(tr);
    }else{
        auto& normalData = *data.n;
        glm::vec3& _position = *normalData.position;
        glm::mat4& _matrix = *normalData.modelMatrix;
        _position.x = x; _position.y = y; _position.z = z;
        _matrix[3][0] = x;
        _matrix[3][1] = y;
        _matrix[3][2] = z;
    }
}
void ComponentBody::setRotation(glm::quat newRotation) { ComponentBody::setRotation(newRotation.x, newRotation.y, newRotation.z, newRotation.w); }
void ComponentBody::setRotation(float x, float y, float z, float w) {
    if (_physics) {
        auto& physicsData = *data.p;
        btQuaternion quat(x, y, z, w);
        quat = quat.normalize();

        btTransform tr; tr.setOrigin(physicsData.rigidBody->getWorldTransform().getOrigin());
        tr.setRotation(quat);

        physicsData.rigidBody->setWorldTransform(tr);
        physicsData.rigidBody->setCenterOfMassTransform(tr);
        physicsData.motionState->setWorldTransform(tr);

        Math::recalculateForwardRightUp(physicsData.rigidBody, _forward, _right, _up);

        clearAngularForces();
    }else{
        auto& normalData = *data.n;
        glm::quat newRotation(w, x, y, z);
        newRotation = glm::normalize(newRotation);
        glm::quat& _rotation = *normalData.rotation;
        _rotation = newRotation;
        Math::recalculateForwardRightUp(_rotation, _forward, _right, _up);
    }
}
void ComponentBody::setScale(glm::vec3 newScale) { ComponentBody::setScale(newScale.x, newScale.y, newScale.z); }
void ComponentBody::setScale(float x, float y, float z) {
    if (_physics) {
        auto& physicsData = *data.p;
        if (physicsData.collision) {
            if (physicsData.collision->getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(physicsData.collision->getShape());
                if (compoundShapeCast) {
                    int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                convexHullCast->setLocalScaling(btVector3(x, y, z));
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                triHullCast->setLocalScaling(btVector3(x, y, z));
                            }
                        }
                    }
                }
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& _scale = *normalData.scale;
        _scale.x = x; _scale.y = y; _scale.z = z;
    }
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        //epriv::ComponentInternalFunctionality::CalculateRadius(*models);
    }
}
const btRigidBody* ComponentBody::getBody() const { return data.p->rigidBody; }
glm::vec3 ComponentBody::position() { //theres prob a better way to do this
    if (_physics) {
        glm::mat4 m(1.0f);
        btTransform tr;  data.p->rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        return glm::vec3(m[3][0], m[3][1], m[3][2]);
    }
    glm::mat4& _matrix = *data.n->modelMatrix;
    return glm::vec3(_matrix[3][0], _matrix[3][1], _matrix[3][2]);
}
glm::vec3 ComponentBody::getScreenCoordinates() { return Math::getScreenCoordinates(position(), false); }
glm::vec3 ComponentBody::getScale() {
    if (_physics) {
        auto& physicsData = *data.p;
        if (physicsData.collision) {
            if (physicsData.collision->getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(physicsData.collision->getShape());
                if (compoundShapeCast) {
                    int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                return Math::btVectorToGLM(const_cast<btVector3&>(convexHullCast->getLocalScaling()));
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                return Math::btVectorToGLM(const_cast<btVector3&>(triHullCast->getLocalScaling()));
                            }
                        }
                    }
                }
            }
        }
        return glm::vec3(1.0f);
    }
    return *data.n->scale;
}
glm::quat ComponentBody::rotation() {
    if (_physics) {
        return Engine::Math::btToGLMQuat(data.p->rigidBody->getWorldTransform().getRotation());
    }
    return *data.n->rotation;
}
glm::vec3 ComponentBody::forward() { return _forward; }
glm::vec3 ComponentBody::right() { return _right; }
glm::vec3 ComponentBody::up() { return _up; }
glm::vec3 ComponentBody::getLinearVelocity() { btVector3 v = data.p->rigidBody->getLinearVelocity(); return Engine::Math::btVectorToGLM(v); }
glm::vec3 ComponentBody::getAngularVelocity() { btVector3 v = data.p->rigidBody->getAngularVelocity(); return Engine::Math::btVectorToGLM(v); }
float ComponentBody::mass() { return data.p->mass; }
glm::mat4 ComponentBody::modelMatrix() { //theres prob a better way to do this
    if (_physics) {
        auto& physicsData = *data.p;
        glm::mat4 m(1.0f);
        btTransform tr;  physicsData.rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        if (physicsData.collision) {
            m = glm::scale(m, getScale());
        }
        return m;
    }
    return *data.n->modelMatrix;
}
void ComponentBody::setDamping(float linear, float angular) { data.p->rigidBody->setDamping(linear, angular); }
void ComponentBody::setDynamic(bool dynamic) {
    if (_physics) {
        auto& physicsData = *data.p;
        if (dynamic) {
            Physics::removeRigidBody(physicsData.rigidBody);
            physicsData.rigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            Physics::addRigidBody(physicsData.rigidBody);
            physicsData.rigidBody->activate();
        }else{
            Physics::removeRigidBody(physicsData.rigidBody);
            physicsData.rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            Physics::addRigidBody(physicsData.rigidBody);
            physicsData.rigidBody->activate();
        }
    }
}
void ComponentBody::setLinearVelocity(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody->setLinearVelocity(v);
    }
}
void ComponentBody::setLinearVelocity(glm::vec3 velocity, bool local) { ComponentBody::setLinearVelocity(velocity.x, velocity.y, velocity.z, local); }
void ComponentBody::setAngularVelocity(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody->setAngularVelocity(v);
    }
}
void ComponentBody::setAngularVelocity(glm::vec3 velocity, bool local) { ComponentBody::setAngularVelocity(velocity.x, velocity.y, velocity.z, local); }
void ComponentBody::applyForce(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody->applyCentralForce(v);
    }
}
void ComponentBody::applyForce(glm::vec3 force, glm::vec3 origin, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(force.x, force.y, force.z);
        if (local) {
            btQuaternion q = physicsData.rigidBody->getWorldTransform().getRotation().normalize();
            v = v.rotate(q.getAxis(), q.getAngle());
        }
        physicsData.rigidBody->applyForce(v, btVector3(origin.x, origin.y, origin.z));
    }
}
void ComponentBody::applyImpulse(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody->applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(glm::vec3 impulse, glm::vec3 origin, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 v(impulse.x, impulse.y, impulse.z);
        if (local) {
            btQuaternion q = physicsData.rigidBody->getWorldTransform().getRotation().normalize();
            v = v.rotate(q.getAxis(), q.getAngle());
        }
        physicsData.rigidBody->applyImpulse(v, btVector3(origin.x, origin.y, origin.z));
    }
}
void ComponentBody::applyTorque(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 t(x, y, z);
        if (local) {
            t = physicsData.rigidBody->getInvInertiaTensorWorld().inverse() * (physicsData.rigidBody->getWorldTransform().getBasis() * t);
        }
        physicsData.rigidBody->applyTorque(t);
    }
}
void ComponentBody::applyTorque(glm::vec3 torque, bool local) { ComponentBody::applyTorque(torque.x, torque.y, torque.z, local); }
void ComponentBody::applyTorqueImpulse(float x, float y, float z, bool local) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->activate();
        btVector3 t(x, y, z);
        if (local) {
            t = physicsData.rigidBody->getInvInertiaTensorWorld().inverse() * (physicsData.rigidBody->getWorldTransform().getBasis() * t);
        }
        physicsData.rigidBody->applyTorqueImpulse(t);
    }
}
void ComponentBody::applyTorqueImpulse(glm::vec3 torqueImpulse, bool local) { ComponentBody::applyTorqueImpulse(torqueImpulse.x, torqueImpulse.y, torqueImpulse.z, local); }
void ComponentBody::clearLinearForces() {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->setActivationState(0);
        physicsData.rigidBody->activate();
        physicsData.rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAngularForces() {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->setActivationState(0);
        physicsData.rigidBody->activate();
        physicsData.rigidBody->setAngularVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAllForces() {
    if (_physics) {
        auto& physicsData = *data.p;
        btVector3 v(0, 0, 0);
        physicsData.rigidBody->setActivationState(0);
        physicsData.rigidBody->activate();
        physicsData.rigidBody->setLinearVelocity(v);
        physicsData.rigidBody->setAngularVelocity(v);
    }
}
void ComponentBody::setMass(float mass) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.mass = mass;
        if (physicsData.collision) {
            physicsData.collision->setMass(physicsData.mass);
            if (physicsData.rigidBody) {
                physicsData.rigidBody->setMassProps(physicsData.mass, physicsData.collision->getInertia());
            }
        }
    }
}
#include "Engine.h"
#include "ObjectDynamic.h"
#include "ObjectDisplay.h"
#include "ShaderProgram.h"
#include "Engine_Renderer.h"
#include "Engine_Physics.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include "Engine_Resources.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace Engine;
using namespace std;

struct DefaultObjectDynamicBindFunctor{void operator()(BindableResource* r) const {
    ObjectDynamic* o = (ObjectDynamic*)r;

    Renderer::sendUniform4f("Object_Color",o->getColor());
    Renderer::sendUniform3f("Gods_Rays_Color",o->getGodsRaysColor());
}};
struct DefaultObjectDynamicUnbindFunctor{void operator()(BindableResource* r) const {
	//ObjectDynamic* o = (ObjectDynamic*)r;
}};
DefaultObjectDynamicBindFunctor DEFAULT_BIND_FUNCTOR;
DefaultObjectDynamicUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

void ObjectDynamic::setDynamic(bool dynamic){
    if(dynamic){
        Physics::removeRigidBody(this);
        m_RigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
        Physics::addRigidBody(this);
        m_RigidBody->activate();
    }
    else{
        Physics::removeRigidBody(this);
        m_RigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        clearAllForces();
        Physics::addRigidBody(this);
        m_RigidBody->activate();
    }
}
void ObjectDynamic::collisionResponse(ObjectDynamic* other){
    // inherit this virtual function for derived classes for collision detection. 
    // if this collides with other, execute this function
}

ObjectDynamic::ObjectDynamic(Handle mesh,Handle mat, glm::vec3 pos, glm::vec3 scl,string n,Collision* col,Scene* scene):Object(n,scene){
    m_Forward = glm::vec3(0,0,-1); m_Right = glm::vec3(1,0,0); m_Up = glm::vec3(0,1,0); m_Color = glm::vec4(1); m_GodsRaysColor = glm::vec3(0);
	m_PassedRenderCheck = false;
    m_Radius = 0;
    m_Visible = true;
    m_BoundingBoxRadius = glm::vec3(0.0f);
	if(!mesh.null() && !mat.null()){
        MeshInstance* item = new MeshInstance(name(),mesh,mat);
        m_MeshInstances.push_back(item);
    }
    m_Collision = col;
    calculateRadius();
    m_Mass = 0.5f * m_Radius;
    if(m_Collision == nullptr){
        if(m_MeshInstances.size() > 0){
            btCompoundShape* shape = new btCompoundShape();
            for(auto meshInstance:m_MeshInstances){
                btTransform t;
                t.setFromOpenGLMatrix(glm::value_ptr(meshInstance->model()));
                shape->addChildShape(t,meshInstance->mesh()->getCollision()->getCollisionShape());
            }
            m_Collision = new Collision(shape,CollisionType::Compound, m_Mass);
        }
    }

    btTransform tr;
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model,pos);
    m_Model *= glm::mat4_cast(glm::quat());
    m_Model = glm::scale(m_Model,glm::vec3(1.0f));

    tr.setFromOpenGLMatrix(glm::value_ptr(m_Model));

    m_MotionState = new btDefaultMotionState(tr);

    calculateRadius();
    m_Mass = 0.5f * m_Radius;
    if(m_Collision == nullptr){
		m_Collision = new Collision(new btEmptyShape(),CollisionType::None,0.0f);
    }
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(m_Mass,m_MotionState,m_Collision->getCollisionShape(),*(m_Collision->getInertia()));
    m_RigidBody = new btRigidBody(rigidBodyCI);

    m_RigidBody->setSleepingThresholds(0.015f,0.015f);
    m_RigidBody->setFriction(0.3f);
    m_RigidBody->setDamping(0.1f,0.4f);//this makes the objects slowly slow down in space, like air friction

    if(Resources::getCurrentScene() == scene || scene == nullptr)
        Physics::addRigidBody(m_RigidBody);

    if(m_Parent == nullptr){ ObjectDynamic::update(0); }

    m_Collision->getCollisionShape()->setUserPointer(this);
    m_RigidBody->setUserPointer(this);

    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
ObjectDynamic::~ObjectDynamic(){
    Physics::removeRigidBody(m_RigidBody);
    SAFE_DELETE(m_RigidBody);
    SAFE_DELETE(m_MotionState);
}
void ObjectDynamic::translate(float x,float y,float z,bool local){
    m_RigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Math::translate(this,vec,local);
    setPosition(getPosition() + glm::vec3(vec.x(),vec.y(),vec.z()));
}
void ObjectDynamic::setColor(float r, float g, float b, float a){ Math::setColor(m_Color,r,g,b,a); }
void ObjectDynamic::setColor(glm::vec4 color){ ObjectDynamic::setColor(color.r,color.g,color.b,color.a); }
void ObjectDynamic::translate(glm::vec3 t,bool l){ ObjectDynamic::translate(t.x,t.y,t.z,l); }
void ObjectDynamic::update(float dt){
    m_Model = glm::mat4(1.0f);

    btTransform tr;
    m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m_Model));

    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    m_Model = glm::scale(m_Model,glm::vec3(localScale.x(),localScale.y(),localScale.z()));

    Engine::Math::recalculateForwardRightUp(m_RigidBody,m_Forward,m_Right,m_Up);

    if(m_Parent != nullptr){
        m_Model =  m_Parent->getModel() * m_Model;
    }
    for(auto meshInstance:m_MeshInstances){
        meshInstance->update(dt);
    }
}
void ObjectDynamic::checkRender(Camera* c){
    if(!m_Visible || !c->sphereIntersectTest(getPosition(),m_Radius) || c->getDistance(this) > m_Radius * Object::m_VisibilityThreshold){
        m_PassedRenderCheck = false;
		return;
    }
    m_PassedRenderCheck = true;
}
glm::vec3 ObjectDynamic::getPosition(){
    glm::mat4 m(1);
    btTransform tr; m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));
    return glm::vec3(m[3][0],m[3][1],m[3][2]);
}
void ObjectDynamic::scale(float x,float y,float z){
    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    m_Collision->getCollisionShape()->setLocalScaling(btVector3(localScale.x()+x,localScale.y()+y,localScale.z()+z));
    this->calculateRadius();
}
void ObjectDynamic::scale(glm::vec3 s){ ObjectDynamic::scale(s.x,s.y,s.z); }
void ObjectDynamic::setScale(float x, float y, float z){
    m_Collision->getCollisionShape()->setLocalScaling(btVector3(x,y,z));
    this->calculateRadius();
}
void ObjectDynamic::setScale(glm::vec3 s){ ObjectDynamic::setScale(s.x,s.y,s.z); }
void ObjectDynamic::setPosition(float x,float y,float z){
    btTransform tr;
    tr.setOrigin(btVector3(btScalar(x),btScalar(y),btScalar(z)));
    tr.setRotation(m_RigidBody->getOrientation());
    
    if(m_Collision->getCollisionType() == CollisionType::TriangleShapeStatic){
        Physics::removeRigidBody(m_RigidBody);
        SAFE_DELETE(m_RigidBody);
    }
    m_MotionState->setWorldTransform(tr);
    if(m_Collision->getCollisionType() == CollisionType::TriangleShapeStatic){
        btRigidBody::btRigidBodyConstructionInfo ci(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
        m_RigidBody = new btRigidBody(ci);
        m_RigidBody->setUserPointer(this);
        Physics::addRigidBody(m_RigidBody);
    }
    m_RigidBody->setWorldTransform(tr);
    m_RigidBody->setCenterOfMassTransform(tr);
}
void ObjectDynamic::setPosition(glm::vec3 p){ ObjectDynamic::setPosition(p.x,p.y,p.z); }
void ObjectDynamic::setOrientation(glm::quat q){
    btTransform t;
    btQuaternion quat;

    quat.setX(q.x);
    quat.setY(q.y);
    quat.setZ(q.z);
    quat.setW(q.w);
    quat = quat.normalize();

    t.setOrigin(m_RigidBody->getWorldTransform().getOrigin());
    t.setRotation(quat);

    m_RigidBody->setWorldTransform(t);
    m_RigidBody->setCenterOfMassTransform(t);
    m_MotionState->setWorldTransform(t);

    Engine::Math::recalculateForwardRightUp(m_RigidBody,m_Forward,m_Right,m_Up);
    clearAngularForces();
}
glm::vec3 ObjectDynamic::getForward(){ return m_Forward; }
glm::vec3 ObjectDynamic::getRight(){ return m_Right; }
glm::vec3 ObjectDynamic::getUp(){ return m_Up; }
void ObjectDynamic::applyForce(float x,float y,float z,bool local){ 
    m_RigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Math::translate(this,vec,local);
    m_RigidBody->applyCentralForce(vec); 
}
void ObjectDynamic::applyForce(glm::vec3 force,glm::vec3 relPos,bool local){ 
    m_RigidBody->activate();
    btVector3 vec = btVector3(force.x,force.y,force.z);
    Math::translate(this,vec,local);
    m_RigidBody->applyForce(vec,btVector3(relPos.x,relPos.y,relPos.z)); 
}
void ObjectDynamic::applyForceX(float x,bool l){ ObjectDynamic::applyForce(x,0,0,l); }
void ObjectDynamic::applyForceY(float y,bool l){ ObjectDynamic::applyForce(0,y,0,l); }
void ObjectDynamic::applyForceZ(float z,bool l){ ObjectDynamic::applyForce(0,0,z,l); }
void ObjectDynamic::applyImpulse(float x,float y,float z,bool local){ 
    m_RigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Math::translate(this,vec,local);
    m_RigidBody->applyCentralImpulse(vec);
}
void ObjectDynamic::applyImpulse(glm::vec3 impulse,glm::vec3 relPos,bool local){ 
    m_RigidBody->activate();
    btVector3 vec = btVector3(impulse.x,impulse.y,impulse.z);
    Math::translate(this,vec,local);
    m_RigidBody->applyImpulse(vec,btVector3(relPos.x,relPos.y,relPos.z));
}
void ObjectDynamic::applyImpulseX(float x,bool l){ ObjectDynamic::applyImpulse(x,0,0,l); }
void ObjectDynamic::applyImpulseY(float y,bool l){ ObjectDynamic::applyImpulse(0,y,0,l); }
void ObjectDynamic::applyImpulseZ(float z,bool l){ ObjectDynamic::applyImpulse(0,0,z,l); }
void ObjectDynamic::applyTorque(float x,float y,float z,bool local){
    m_RigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*t);
    }
    m_RigidBody->applyTorque(t);
}
void ObjectDynamic::applyTorque(glm::vec3 t,bool l){ ObjectDynamic::applyTorque(t.x,t.y,t.z,l); }
void ObjectDynamic::applyTorqueX(float x,bool l){ ObjectDynamic::applyTorque(x,0,0,l); }
void ObjectDynamic::applyTorqueY(float y,bool l){ ObjectDynamic::applyTorque(0,y,0,l); }
void ObjectDynamic::applyTorqueZ(float z,bool l){ ObjectDynamic::applyTorque(0,0,z,l); }
void ObjectDynamic::applyTorqueImpulse(float x,float y,float z,bool local){
    m_RigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*t);
    }
    m_RigidBody->applyTorqueImpulse(t);
}
void ObjectDynamic::applyTorqueImpulse(glm::vec3 t,bool l){ ObjectDynamic::applyTorqueImpulse(t.x,t.y,t.z,l); }
void ObjectDynamic::applyTorqueImpulseX(float x,bool l){ ObjectDynamic::applyTorqueImpulse(x,0,0,l); }
void ObjectDynamic::applyTorqueImpulseY(float y,bool l){ ObjectDynamic::applyTorqueImpulse(0,y,0,l); }
void ObjectDynamic::applyTorqueImpulseZ(float z,bool l){ ObjectDynamic::applyTorqueImpulse(0,0,z,l); }
void ObjectDynamic::setLinearVelocity(float x, float y, float z, bool local){
    m_RigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Math::translate(this,vec,local);
    m_RigidBody->setLinearVelocity(vec); 
}
void ObjectDynamic::setLinearVelocity(glm::vec3 v, bool l){ ObjectDynamic::setLinearVelocity(v.x,v.y,v.z,l); }
void ObjectDynamic::setLinearVelocityX(float x, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    ObjectDynamic::setLinearVelocity(x,v.y(),v.z(),l); 
}
void ObjectDynamic::setLinearVelocityY(float y, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    ObjectDynamic::setLinearVelocity(v.x(),y,v.z(),l); 
}
void ObjectDynamic::setLinearVelocityZ(float z, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    ObjectDynamic::setLinearVelocity(v.x(),v.y(),z,l); 
}
void ObjectDynamic::setAngularVelocity(float x, float y, float z,bool local){ 
    m_RigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Math::translate(this,vec,local);
    m_RigidBody->setAngularVelocity(vec); 
}
void ObjectDynamic::setAngularVelocity(glm::vec3 v,bool l){ ObjectDynamic::setAngularVelocity(v.x,v.y,v.z,l); }
void ObjectDynamic::setAngularVelocityX(float x, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    ObjectDynamic::setAngularVelocity(x,v.y(),v.z(),l); 
}
void ObjectDynamic::setAngularVelocityY(float y, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    ObjectDynamic::setAngularVelocity(v.x(),y,v.z(),l); 
}
void ObjectDynamic::setAngularVelocityZ(float z, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    ObjectDynamic::setAngularVelocity(v.x(),v.y(),z,l); 
}
void ObjectDynamic::setMass(float mass){
    m_Mass = mass;
    m_Collision->setMass(m_Mass);
    if(m_RigidBody != nullptr){
        m_RigidBody->setMassProps(m_Mass,*(m_Collision->getInertia()));
    }
}
void ObjectDynamic::lookAt(glm::vec3 eye,glm::vec3 target,glm::vec3 up){
    btQuaternion btQ = m_RigidBody->getOrientation();
    glm::quat q = Engine::Math::btToGLMQuat(btQ);

    m_RigidBody->getWorldTransform().setRotation(btQ);

    Engine::Math::lookAtToQuat(q,eye,target,up);

    btQ = Engine::Math::glmToBTQuat(q);
    m_RigidBody->getWorldTransform().setRotation(btQ);

    Engine::Math::recalculateForwardRightUp(m_RigidBody,m_Forward,m_Right,m_Up);
}
void ObjectDynamic::lookAt(Object* o){ ObjectDynamic::lookAt(getPosition(), o->getPosition(), o->getUp()); }
void ObjectDynamic::alignTo(glm::vec3 direction, float speed){
    ObjectDynamic::clearAngularForces();
    btQuaternion _q = m_RigidBody->getWorldTransform().getRotation();
    glm::quat q;
    q.x = _q.x();
    q.y = _q.y();
    q.z = _q.z();
    q.w = _q.w();
    Engine::Math::alignTo(q,this,direction, speed);
    _q.setX(q.x);
    _q.setY(q.y);
    _q.setZ(q.z);
    _q.setW(q.w);
    _q = _q.normalize();
    m_RigidBody->getWorldTransform().setRotation(_q);
}
void ObjectDynamic::alignTo(Object* other, float speed){
    glm::vec3 direction = getPosition() - other->getPosition();
    ObjectDynamic::alignTo(direction,speed);
}
void ObjectDynamic::alignToX(Object* other, float speed){
    glm::vec3 direction = getPosition() - other->getPosition();
    ObjectDynamic::clearAngularForces();
    btQuaternion btQ = m_RigidBody->getOrientation();
    glm::quat q = Engine::Math::btToGLMQuat(btQ);
    Engine::Math::alignToX(q,this,other, speed);
    btQ = Engine::Math::glmToBTQuat(q);
    m_RigidBody->getWorldTransform().setRotation(btQ);
}
void ObjectDynamic::alignToY(Object* other, float speed){
    glm::vec3 direction = getPosition() - other->getPosition();
    ObjectDynamic::clearAngularForces();
    btQuaternion btQ = m_RigidBody->getOrientation();
    glm::quat q = Engine::Math::btToGLMQuat(btQ);
    Engine::Math::alignToY(q,this,other, speed);
    btQ = Engine::Math::glmToBTQuat(q);
    m_RigidBody->getWorldTransform().setRotation(btQ);
}
void ObjectDynamic::alignToZ(Object* other, float speed){
    glm::vec3 direction = getPosition() - other->getPosition();
    ObjectDynamic::clearAngularForces();
    btQuaternion btQ = m_RigidBody->getOrientation();
    glm::quat q = Engine::Math::btToGLMQuat(btQ);
    Engine::Math::alignToZ(q,this,other, speed);
    btQ = Engine::Math::glmToBTQuat(q);
    m_RigidBody->getWorldTransform().setRotation(btQ);
}
void ObjectDynamic::rotate(float x,float y,float z,bool overTime){
    ObjectDynamic::clearAngularForces();
    if(overTime){
        x *= Resources::dt(); y *= Resources::dt(); z *= Resources::dt();
    }
    if(abs(x) < Object::m_RotationThreshold && abs(y) < Object::m_RotationThreshold && abs(z) < Object::m_RotationThreshold)
        return;

    if(abs(x) >= Object::m_RotationThreshold) this->applyTorqueY(-x);  //pitch
    if(abs(y) >= Object::m_RotationThreshold) this->applyTorqueX(-y);  //yaw
    if(abs(z) >= Object::m_RotationThreshold) this->applyTorqueZ(z);   //roll

    Engine::Math::recalculateForwardRightUp(m_RigidBody,m_Forward,m_Right,m_Up);
}
void ObjectDynamic::rotate(glm::vec3 r, bool overTime){ ObjectDynamic::rotate(r.x,r.y,r.z,overTime); }
void ObjectDynamic::clearLinearForces(){
    m_RigidBody->setActivationState(0);
    ObjectDynamic::setLinearVelocity(0,0,0);
}
void ObjectDynamic::clearAngularForces(){
    m_RigidBody->setActivationState(0);
    ObjectDynamic::setAngularVelocity(0,0,0);
}
void ObjectDynamic::clearAllForces(){
    m_RigidBody->setActivationState(0);
    ObjectDynamic::setLinearVelocity(0,0,0);
    ObjectDynamic::setAngularVelocity(0,0,0);
}
bool ObjectDynamic::rayIntersectSphere(Camera* c){
    if(c == nullptr) c = Resources::getCurrentScene()->getActiveCamera();
    return c->rayIntersectSphere(this);
}
void ObjectDynamic::calculateRadius(){
	if(m_MeshInstances.size() == 0){
        m_BoundingBoxRadius = glm::vec3(0.0f);
        m_Radius = 0;
        return;
    }
    float maxLength = 0;
    for(auto meshInstance:m_MeshInstances){
        float length = 0;
        glm::mat4 m = meshInstance->model();
        glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
        length = glm::length(localPosition) + meshInstance->mesh()->getRadius() * Engine::Math::Max(meshInstance->getScale());
        if(length > maxLength){
            maxLength = length;
        }
    }
    glm::vec3 scale(1.0f);
    if(m_Collision != nullptr){
        btVector3 s = m_Collision->getCollisionShape()->getLocalScaling();
        scale = glm::vec3(s.x(),s.y(),s.z());
    }
    m_BoundingBoxRadius = maxLength * scale;
    m_Radius = Engine::Math::Max(m_BoundingBoxRadius);
}
bool ObjectDynamic::rayIntersectSphere(glm::vec3 A, glm::vec3 rayVector){
    return Engine::Math::rayIntersectSphere(getPosition(),getRadius(),A,rayVector);
}
glm::quat& ObjectDynamic::getOrientation(){
    btTransform t;
    m_RigidBody->getMotionState()->getWorldTransform(t);
    btQuaternion q = t.getRotation().normalize();
    glm::quat _q;
    _q.x = q.x();
    _q.y = q.y();
    _q.z = q.z();
    _q.w = q.w();
    return _q;
}
glm::vec3 ObjectDynamic::getScale(){
    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    return glm::vec3(localScale.x(),localScale.y(),localScale.z());
}
glm::mat4& ObjectDynamic::getModel(){ return m_Model; }

void ObjectDynamic::suspend(){
    Physics::removeRigidBody(this);
    for(auto meshInstance:m_MeshInstances){
        if(meshInstance->mesh() != nullptr){
            meshInstance->mesh()->decrementUseCount();
            if(meshInstance->mesh()->useCount() == 0){
                meshInstance->mesh()->unload();
            }
        }
        if(meshInstance->material() != nullptr){
            meshInstance->material()->decrementUseCount();
            if(meshInstance->material()->useCount() == 0){
                meshInstance->material()->unload();
            }
        }
    }
}
void ObjectDynamic::resume(){
    Physics::addRigidBody(this);
    for(auto meshInstance:m_MeshInstances){
        if(meshInstance->mesh() != nullptr){
            meshInstance->mesh()->incrementUseCount();
            meshInstance->mesh()->load();
        }
        if(meshInstance->material() != nullptr){
            meshInstance->material()->incrementUseCount();
            meshInstance->material()->load();
        }
    }
}

void ObjectDynamic::setMesh(Mesh* mesh){
    Physics::removeRigidBody(this);
    for(auto meshInstance:m_MeshInstances){ 
        meshInstance->setMesh(mesh); 
    } 
    if(m_MeshInstances.size() > 0){
        if(m_Collision != nullptr){
			epriv::Core::m_Engine->m_PhysicsManager->_removeCollision(m_Collision);
        }
        btCompoundShape* shape = new btCompoundShape();
        for(auto meshInstance:m_MeshInstances){
            btTransform t;
            t.setFromOpenGLMatrix(glm::value_ptr(meshInstance->model()));
            shape->addChildShape(t,meshInstance->mesh()->getCollision()->getCollisionShape());
        }
        calculateRadius();
        m_Mass = 0.5f * m_Radius;
        m_Collision = new Collision(shape,CollisionType::Compound, m_Mass);
        m_RigidBody->setCollisionShape(m_Collision->getCollisionShape());
        setMass(m_Mass);
    }
    Physics::addRigidBody(this);
}
void ObjectDynamic::setMesh(Handle& meshHandle){ 
	ObjectDynamic::setMesh(Resources::getMesh(meshHandle));
}
void ObjectDynamic::setMaterial(Material* material){ 
	for(auto meshInstance:m_MeshInstances){ 
		meshInstance->setMaterial(material);
	} 
}
void ObjectDynamic::setMaterial(Handle& material){ 
	for(auto meshInstance:m_MeshInstances){ 
		meshInstance->setMaterial(Resources::getMaterial(material)); 
	}
}

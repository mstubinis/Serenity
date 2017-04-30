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

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;


struct DefaultObjectDynamicBindFunctor{void operator()(BindableResource* r) const {
	ObjectDynamic* o = static_cast<ObjectDynamic*>(r);

    Renderer::sendUniform4f("Object_Color",o->getColor());
    Renderer::sendUniform3f("Gods_Rays_Color",o->getGodsRaysColor());
}};
struct DefaultObjectDynamicUnbindFunctor{void operator()(BindableResource* r) const {

}};
DefaultObjectDynamicBindFunctor ObjectDynamic::DEFAULT_BIND_FUNCTOR;
DefaultObjectDynamicUnbindFunctor ObjectDynamic::DEFAULT_UNBIND_FUNCTOR;

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

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::v3 pos, glm::vec3 scl, std::string n,Collision* col,Scene* scene): Object(n,scene){
    m_Forward = glm::v3(0,0,-1);
    m_Right = glm::v3(1,0,0);
    m_Up = glm::v3(0,1,0);
    m_Radius = 0;
    m_Visible = true;
    m_BoundingBoxRadius = glm::vec3(0);
    if(mesh != "" && mat != ""){
        RenderedItem* item = new RenderedItem(name(),mesh,mat);
        m_DisplayItems.push_back(item);
    }
    m_Color = glm::vec4(1);
    m_GodsRaysColor = glm::vec3(0);
    m_Collision = col;
    calculateRadius();
    m_Mass = 0.5f * m_Radius;
    if(m_Collision == nullptr){
        if(m_DisplayItems.size() > 0){
            btCompoundShape* shape = new btCompoundShape();
            for(auto item:m_DisplayItems){
                btTransform t;
                t.setFromOpenGLMatrix(glm::value_ptr(item->model()));
                shape->addChildShape(t,item->mesh()->getCollision()->getCollisionShape());
            }
            m_Collision = new Collision(shape,COLLISION_TYPE_COMPOUND, m_Mass);
        }
    }

    btTransform tr;
    m_Model = glm::m4(1);
    m_Model = glm::translate(m_Model,pos);
    m_Model *= glm::m4(glm::mat4_cast(glm::quat()));
    m_Model = glm::scale(m_Model,glm::v3(glm::vec3(1)));

    tr.setFromOpenGLMatrix(glm::value_ptr(glm::mat4(m_Model)));

    m_MotionState = new btDefaultMotionState(tr);

    calculateRadius();
    m_Mass = 0.5f * m_Radius;

    if(m_Collision != nullptr){
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(m_Mass,m_MotionState,m_Collision->getCollisionShape(),*(m_Collision->getInertia()));
        m_RigidBody = new btRigidBody(rigidBodyCI);
    }
    else{
        m_Collision = new Collision(new btEmptyShape(),COLLISION_TYPE_NONE,0.0f);
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(m_Mass,m_MotionState,m_Collision->getCollisionShape(),*(m_Collision->getInertia()));
        m_RigidBody = new btRigidBody(rigidBodyCI);
    }
    m_RigidBody->setSleepingThresholds(0.015f,0.015f);
    m_RigidBody->setFriction(0.3f);
    m_RigidBody->setDamping(0.1f,0.4f);//this makes the objects slowly slow down in space, like air friction

    if(Resources::getCurrentScene() == scene || scene == nullptr)
        Physics::addRigidBody(m_RigidBody);

    if(m_Parent == nullptr){ ObjectDynamic::update(0); }

    m_Collision->getCollisionShape()->setUserPointer(this);
    m_RigidBody->setUserPointer(this);

    setCustomBindFunctor(ObjectDynamic::DEFAULT_BIND_FUNCTOR);
	setCustomUnbindFunctor(ObjectDynamic::DEFAULT_UNBIND_FUNCTOR);
}
ObjectDynamic::~ObjectDynamic(){
    Physics::removeRigidBody(m_RigidBody);
    SAFE_DELETE(m_RigidBody);
    SAFE_DELETE(m_MotionState);
}
void ObjectDynamic::translate(glm::num x, glm::num y, glm::num z,bool local){
    m_RigidBody->activate();
    btTransform t = m_RigidBody->getWorldTransform();
    btVector3 pos = t.getOrigin();
    glm::v3 p = glm::v3(pos.x(),pos.y(),pos.z());
    if(local){
        p += getForward() * z;
        p += getRight() * x;
        p += getUp() * y;
    }
    else{
        p += glm::vec3(x,y,z);
    }
    setPosition(getPosition() + p);
}
void ObjectDynamic::setColor(float r, float g, float b, float a){
    Math::setColor(m_Color,r,g,b,a);
}
void ObjectDynamic::setColor(glm::vec4 color){ ObjectDynamic::setColor(color.r,color.g,color.b,color.a); }
void ObjectDynamic::translate(glm::v3 t,bool l){ ObjectDynamic::translate(t.x,t.y,t.z,l); }
void ObjectDynamic::update(float dt){
    glm::mat4 m(1);

    btTransform tr;
    m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));

    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    m = glm::scale(m,glm::vec3(localScale.x(),localScale.y(),localScale.z()));

    m_Forward = Engine::Math::getForward(m_RigidBody);
    m_Right = Engine::Math::getRight(m_RigidBody);
    m_Up = Engine::Math::getUp(m_RigidBody);

    m_Model = glm::m4(m);
    if(m_Parent != nullptr){
        m_Model =  m_Parent->getModel() * m_Model;
    }
    for(auto renderedItem:m_DisplayItems){
        renderedItem->update(dt);
    }
    Camera* c = Resources::getActiveCamera();
    m_PassedRenderCheck = true;
    if(!m_Visible || !c->sphereIntersectTest(this) || c->getDistance(this) > m_Radius * 1100.0f){
        m_PassedRenderCheck = false;
    }
}
glm::v3 ObjectDynamic::getPosition(){
    glm::mat4 m(1);
    btTransform tr; m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));
    return glm::v3(m[3][0],m[3][1],m[3][2]);
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
void ObjectDynamic::setPosition(glm::num x, glm::num y, glm::num z){
    btTransform tr;
    tr.setOrigin(btVector3(btScalar(x),btScalar(y),btScalar(z)));
    tr.setRotation(m_RigidBody->getOrientation());
    
    if(m_Collision->getCollisionType() == COLLISION_TYPE_STATIC_TRIANGLESHAPE){
        Physics::removeRigidBody(m_RigidBody);
        SAFE_DELETE(m_RigidBody);
    }
    m_MotionState->setWorldTransform(tr);
    if(m_Collision->getCollisionType() == COLLISION_TYPE_STATIC_TRIANGLESHAPE){
        btRigidBody::btRigidBodyConstructionInfo ci(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
        m_RigidBody = new btRigidBody(ci);
        m_RigidBody->setUserPointer(this);
        Physics::addRigidBody(m_RigidBody);
    }
    m_RigidBody->setWorldTransform(tr);
    m_RigidBody->setCenterOfMassTransform(tr);
}
void ObjectDynamic::setOrientation(glm::quat q){
    btTransform t;
    btQuaternion quat(q.x,q.y,q.z,q.w);

    t.setOrigin(m_RigidBody->getWorldTransform().getOrigin());
    t.setRotation(quat);

    m_RigidBody->setWorldTransform(t);
    m_RigidBody->setCenterOfMassTransform(t);
    m_MotionState->setWorldTransform(t);

    clearAngularForces();
}
void ObjectDynamic::setPosition(glm::v3 p){ ObjectDynamic::setPosition(p.x,p.y,p.z); }
void ObjectDynamic::applyForce(float x,float y,float z,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * x;
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getForward()) * z;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->applyCentralForce(btVector3(x,y,z)); 
}
void ObjectDynamic::applyForce(glm::vec3 force,glm::vec3 relPos,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * force.x;
        res += glm::vec3(getUp()) * force.y;
        res += glm::vec3(getForward()) * force.z;
        force.x = res.x; force.y = res.y; force.z = res.z;
    }
    m_RigidBody->applyForce(btVector3(force.x,force.y,force.z),btVector3(relPos.x,relPos.y,relPos.z)); 
}
void ObjectDynamic::applyForceX(float x,bool l){ ObjectDynamic::applyForce(x,0,0,l); }
void ObjectDynamic::applyForceY(float y,bool l){ ObjectDynamic::applyForce(0,y,0,l); }
void ObjectDynamic::applyForceZ(float z,bool l){ ObjectDynamic::applyForce(0,0,z,l); }
void ObjectDynamic::applyImpulse(float x,float y,float z,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getForward()) * z; 
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getRight()) * x;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->applyCentralImpulse(btVector3(x,y,z));
}
void ObjectDynamic::applyImpulse(glm::vec3 impulse,glm::vec3 relPos,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getForward()) * impulse.z; 
        res += glm::vec3(getUp()) * impulse.y;
        res += glm::vec3(getRight()) * impulse.x;
        impulse.x = res.x; impulse.y = res.y; impulse.z = res.z;
    }
    m_RigidBody->applyImpulse(btVector3(impulse.x,impulse.y,impulse.z),btVector3(relPos.x,relPos.y,relPos.z));
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
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * x;
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getForward()) * z;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->setLinearVelocity(btVector3(x,y,z)); 
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
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * x;
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getForward()) * z;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
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
    if(m_RigidBody != nullptr)
        m_RigidBody->setMassProps(m_Mass,*(m_Collision->getInertia()));
}
void ObjectDynamic::alignTo(glm::v3 direction, float speed){
    ObjectDynamic::clearAngularForces();
    btQuaternion btQ = m_RigidBody->getOrientation();
	glm::quat q = Engine::Math::btToGLMQuat(btQ);
    Engine::Math::alignTo(q,this,glm::vec3(direction), speed);
	btQ = Engine::Math::glmToBTQuat(q);
    m_RigidBody->getWorldTransform().setRotation(btQ);
}
void ObjectDynamic::alignTo(Object* other, float speed){
	glm::v3 direction = getPosition() - other->getPosition();
    ObjectDynamic::alignTo(direction,speed);
}
void ObjectDynamic::rotate(float x,float y,float z,bool overTime){
    ObjectDynamic::clearAngularForces();
    if(overTime){
        x *= Resources::dt(); y *= Resources::dt(); z *= Resources::dt();
    }
    float threshold = 0;
    if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
        return;

    if(abs(x) >= threshold) this->applyTorqueY(-x);   //pitch
    if(abs(y) >= threshold) this->applyTorqueX(-y);   //yaw
    if(abs(z) >= threshold) this->applyTorqueZ(z);   //roll

    m_Forward = Engine::Math::getForward(m_RigidBody);
    m_Right = Engine::Math::getRight(m_RigidBody);
    m_Up = Engine::Math::getUp(m_RigidBody);
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
    if(c == nullptr) c = Resources::getActiveCamera();
    return c->rayIntersectSphere(this); 
}
void ObjectDynamic::calculateRadius(){
    if(m_DisplayItems.size() == 0){
        m_BoundingBoxRadius = glm::vec3(0);
        m_Radius = 0;
        return;
    }
    float maxLength = 0;
    for(auto item:m_DisplayItems){
        float length = 0;
        glm::mat4 m = item->model();
        glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
        length = glm::length(localPosition) + item->mesh()->getRadius() * Engine::Math::Max(item->getScale());
        if(length > maxLength){
            maxLength = length;
        }
    }
    glm::vec3 scale(1);
    if(m_Collision != nullptr){
        btVector3 s = m_Collision->getCollisionShape()->getLocalScaling();
        scale = glm::vec3(s.x(),s.y(),s.z());
    }
    m_BoundingBoxRadius = maxLength * scale;
    m_Radius = Engine::Math::Max(m_BoundingBoxRadius);
}
bool ObjectDynamic::rayIntersectSphere(glm::v3 A, glm::vec3 rayVector){
    return Engine::Math::rayIntersectSphere(glm::vec3(getPosition()),getRadius(),A,rayVector);
}
glm::quat& ObjectDynamic::getOrientation(){
    btQuaternion q = m_RigidBody->getOrientation();
	return Engine::Math::btToGLMQuat(q);
}
glm::vec3 ObjectDynamic::getScale(){
    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    return glm::vec3(localScale.x(),localScale.y(),localScale.z());
}
glm::m4& ObjectDynamic::getModel(){ return m_Model; }

#include "ObjectDynamic.h"
#include "Engine_Physics.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include "Engine_Resources.h"
#include "Mesh.h"
#include "Camera.h"

using namespace Engine;

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, std::string name,Engine::Physics::Collision* col,Scene* scene): ObjectDisplay(mesh,mat,pos,scl,name,true,scene){
	m_Collision_Shape = col;
	m_Mass = 0.5f * m_Radius;
	if(m_Collision_Shape == nullptr){
		m_Collision_Shape = new Engine::Physics::Collision(nullptr,COLLISION_TYPE_NONE);
		if(m_Mesh != nullptr){
			if(m_Mesh->getCollision() == nullptr){
				m_Collision_Shape->setCollision(new btBoxShape(btVector3(m_BoundingBoxRadius.x,m_BoundingBoxRadius.y,m_BoundingBoxRadius.z)),COLLISION_TYPE_BOXSHAPE,m_Mass);
			}
			else
				m_Collision_Shape->setCollision(m_Mesh->getCollision()->getCollision(),m_Mesh->getCollision()->getCollisionType(),m_Mass);
		}
	}

	btTransform tr;
	glm::mat4 m = glm::mat4(1);
	m = glm::translate(m,glm::vec3(m_Position));
	m *= glm::mat4_cast(m_Orientation);
	m = glm::scale(m,m_Scale);
	tr.setFromOpenGLMatrix(glm::value_ptr(m));

	m_MotionState = new btDefaultMotionState(tr);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(m_Mass,m_MotionState,m_Collision_Shape->getCollision(),*(m_Collision_Shape->getInertia()));

	m_RigidBody = new btRigidBody(rigidBodyCI);
	m_RigidBody->setSleepingThresholds(0.15f,0.15f);
	m_RigidBody->setFriction(0.3f);
	m_RigidBody->setDamping(0.3f,0.5f);//this makes the objects slowly slow down in space, like air friction
	Physics::addRigidBody(m_RigidBody);

	if(m_Parent == nullptr){
		ObjectDynamic::update(1);
	}
}
ObjectDynamic::~ObjectDynamic(){
	delete m_Collision_Shape;
	delete m_RigidBody;
	delete m_MotionState;
}
void ObjectDynamic::translate(float x, float y, float z,bool local){
	m_RigidBody->activate();
	btTransform transform = m_RigidBody->getWorldTransform();
	btVector3 pos = transform.getOrigin();
	glm::vec3 p = glm::vec3(pos.x(),pos.y(),pos.z());
	if(local){
		p += getForward() * z;
		p += getRight() * x;
		p += getUp() * y;
	}
	else{
		p += glm::vec3(x,y,z);
	}
	this->setPosition(this->getPosition() + p);
}
void ObjectDynamic::translate(glm::vec3 translation,bool local){ translate(translation.x,translation.y,translation.z,local); }
glm::vec3 ObjectDynamic::_calculateForward(){ 
	btTransform t;
	m_RigidBody->getMotionState()->getWorldTransform(t);
	btVector3 m = t.getBasis().getColumn(2);
	return glm::vec3(m.x(),m.y(),m.z());
}
glm::vec3 ObjectDynamic::_calculateRight(){
	btTransform t;
	m_RigidBody->getMotionState()->getWorldTransform(t);
	btVector3 m = t.getBasis().getColumn(0);
	return glm::vec3(m.x(),m.y(),m.z());
}
glm::vec3 ObjectDynamic::_calculateUp(){
	btTransform t;
	m_RigidBody->getMotionState()->getWorldTransform(t);
	btVector3 m = t.getBasis().getColumn(1);
	return glm::vec3(m.x(),m.y(),m.z());
}
void ObjectDynamic::update(float dt){
	glm::mat4 parentModel = glm::mat4(1);
	glm::mat4 newModel = glm::mat4(1);
	btTransform tr;
	m_RigidBody->getMotionState()->getWorldTransform(tr);
	tr.getOpenGLMatrix(glm::value_ptr(newModel));
	if(m_Parent != nullptr){
		parentModel = m_Parent->getModel();
	}
	if(m_RigidBody->isActive()){
		btQuaternion t = tr.getRotation();
		m_Orientation = glm::quat(t.w(),t.x(),t.y(),t.z());

		m_Forward = ObjectDynamic::_calculateForward();
		m_Right = ObjectDynamic::_calculateRight();
		m_Up = ObjectDynamic::_calculateUp();
	}
	m_Model = parentModel * newModel;
	for(auto child:m_Children)
		child->_updateMatrix();
}
void ObjectDynamic::_updateMatrix(){

}
void ObjectDynamic::scale(float x,float y,float z){
	ObjectDisplay::scale(x,y,z);
	this->m_Collision_Shape->getCollision()->setLocalScaling(btVector3(m_Scale.x,m_Scale.y,m_Scale.z));
}
void ObjectDynamic::scale(glm::vec3 scl){ ObjectDynamic::scale(scl.x,scl.y,scl.z); }
void ObjectDynamic::setPosition(float x, float y, float z){
	m_RigidBody->activate();
	btTransform t;
	m_RigidBody->getMotionState()->getWorldTransform(t);
	t.setOrigin(btVector3(x,y,z));
	m_RigidBody->setWorldTransform(t);
}
void ObjectDynamic::setPosition(glm::vec3 p){ ObjectDynamic::setPosition(p.x,p.y,p.z); }
void ObjectDynamic::applyForce(float x,float y,float z,bool local){ 
	m_RigidBody->activate();
	if(local){
		glm::vec3 res = getRight() * x;
		res += getUp() * y;
		res += getForward() * z;
		x = res.x; y = res.y; z = res.z;
	}
	m_RigidBody->applyCentralForce(btVector3(x,y,z)); 
}
void ObjectDynamic::applyForce(glm::vec3 force,glm::vec3 relPos,bool local){ 
	m_RigidBody->activate();
	if(local){
		glm::vec3 res = getRight() * force.x;
		res += getUp() * force.y;
		res += getForward() * force.z;
		force.x = res.x; force.y = res.y; force.z = res.z;
	}
	m_RigidBody->applyForce(btVector3(force.x,force.y,force.z),btVector3(relPos.x,relPos.y,relPos.z)); 
}
void ObjectDynamic::applyImpulse(float x,float y,float z){ 
	m_RigidBody->activate();
	m_RigidBody->applyCentralImpulse(btVector3(x,y,z));
}
void ObjectDynamic::applyImpulse(glm::vec3 impulse,glm::vec3 relPos){ 
	m_RigidBody->activate();
	m_RigidBody->applyImpulse(btVector3(impulse.x,impulse.y,impulse.z),btVector3(relPos.x,relPos.y,relPos.z));
}
void ObjectDynamic::applyTorque(float x,float y,float z){
	m_RigidBody->activate();
	btVector3 t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*btVector3(x,y,z));
	m_RigidBody->applyTorque(t);
}
void ObjectDynamic::applyTorque(glm::vec3 torque){ ObjectDynamic::applyTorque(torque.x,torque.y,torque.z); }
void ObjectDynamic::applyTorqueImpulse(float x,float y,float z){
	m_RigidBody->activate();
	btVector3 t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*btVector3(x,y,z));
	m_RigidBody->applyTorqueImpulse(t);
}
void ObjectDynamic::applyTorqueImpulse(glm::vec3 torque){ ObjectDynamic::applyTorqueImpulse(torque.x,torque.y,torque.z); }
void ObjectDynamic::setLinearVelocity(float x, float y, float z, bool local){
	m_RigidBody->activate();
	if(local){
		glm::vec3 res = getRight() * x;
		res += getUp() * y;
		res += getForward() * z;
		x = res.x; y = res.y; z = res.z;
	}
	m_RigidBody->setLinearVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::setLinearVelocity(glm::vec3 velocity, bool local){ ObjectDynamic::setLinearVelocity(velocity.x,velocity.y,velocity.z,local); }
void ObjectDynamic::setAngularVelocity(float x, float y, float z){ 
	m_RigidBody->activate();
	m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::setAngularVelocity(glm::vec3 velocity){ ObjectDynamic::setAngularVelocity(velocity.x,velocity.y,velocity.z); }
void ObjectDynamic::setMass(float mass){
	m_RigidBody->activate();
	m_Mass = 0.5f * m_Radius;

	m_Collision_Shape->recalculate(m_Mass);

	if(m_RigidBody != nullptr)
		m_RigidBody->setMassProps(m_Mass,*(m_Collision_Shape->getInertia()));
}
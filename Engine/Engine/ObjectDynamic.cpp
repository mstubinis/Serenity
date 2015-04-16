#include "ObjectDynamic.h"
#include "Engine_Physics.h"
#include <Bullet/btBulletCollisionCommon.h>
#include <Bullet/btBulletDynamicsCommon.h>
#include "Engine_Resources.h"
#include "Mesh.h"
#include "Camera.h"

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, std::string name,btCollisionShape* collisionShape): Object(mesh,mat,pos,scl,name,true){
	m_Collision_Shape = collisionShape;
	m_Inertia = new btVector3(0,0,0);
	if(m_Collision_Shape == nullptr){
		if(m_Mesh != nullptr){
			if(m_Mesh->Collision() == nullptr)
				m_Collision_Shape = new btBoxShape(btVector3(m_BoundingBoxRadius.x,m_BoundingBoxRadius.y,m_BoundingBoxRadius.z));
			else
				m_Collision_Shape = m_Mesh->Collision();
		}
	}

	btTransform tr;
	glm::mat4 m = glm::mat4(1);
	m = glm::translate(m,m_Position);
	m *= glm::mat4_cast(m_Orientation);
	m = glm::scale(m,m_Scale);
	tr.setFromOpenGLMatrix(glm::value_ptr(m));

	m_MotionState = new btDefaultMotionState(tr);

	m_Mass = 0.5f * m_Radius;

	m_Collision_Shape->calculateLocalInertia(m_Mass,*m_Inertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(m_Mass,m_MotionState,m_Collision_Shape,*m_Inertia);

	m_RigidBody = new btRigidBody(rigidBodyCI);
	m_RigidBody->setSleepingThresholds(0.15f,0.15f);
	m_RigidBody->setFriction(0.3f);
	m_RigidBody->setDamping(0.3f,0.5f);//this makes the objects slowly slow down in space, like air friction
	physicsEngine->Add_Rigid_Body(m_RigidBody);
}
ObjectDynamic::~ObjectDynamic(){
	delete m_Collision_Shape;
	delete m_RigidBody;
	delete m_MotionState;
	delete m_Inertia;
}
void ObjectDynamic::Translate(float x, float y, float z,bool local){
	x *= Resources->dt; y*= Resources->dt; z*=Resources->dt;
	btVector3 pos = m_RigidBody->getWorldTransform().getOrigin();
	glm::vec3 p = glm::vec3(pos.x(),pos.y(),pos.z());
	if(local){
		p += Forward() * z;
		p += Right() * x;
		p += Up() * y;
	}
	else{
		p += glm::vec3(x,y,z);
	}
	m_RigidBody->getWorldTransform().setOrigin(btVector3(p.x,p.y,p.z));
	Flag_As_Changed();
}
void ObjectDynamic::Translate(glm::vec3& translation,bool local){ Translate(translation.x,translation.y,translation.z); }
glm::vec3 ObjectDynamic::_Forward(){ return glm::normalize(glm::cross(Right(),Up())); }
glm::vec3 ObjectDynamic::_Right(){
	btQuaternion q = m_RigidBody->getCenterOfMassTransform().getRotation();
	float x = q.x();
	float y = q.y();
	float z = q.z();
	float w = q.w();
	return glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),
                                     2 * (x * y + w * z),
                                     2 * (x * z - w * y)));
}
glm::vec3 ObjectDynamic::_Up(){
	btQuaternion q = m_RigidBody->getCenterOfMassTransform().getRotation();
	float x = q.x();
	float y = q.y();
	float z = q.z();
	float w = q.w();
	return glm::normalize(glm::vec3( 2 * (x * y - w * z), 
                                     1 - 2 * (x * x + z * z),
                                     2 * (y * z + w * x)));
}
void ObjectDynamic::Update(float dt){
	if(m_RigidBody->isActive())
		Flag_As_Changed();
	if(m_Changed){ 
		m_RigidBody->activate();
		m_Changed = false;
	}
	glm::mat4 parentModel = glm::mat4(1);
	if(m_Parent != nullptr){
		parentModel = m_Parent->Model();
		m_Orientation = m_Parent->Orientation();
	}
	if(m_RigidBody->isActive()){
		m_RigidBody->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m_Model));
		btQuaternion t = m_RigidBody->getWorldTransform().getRotation();
		m_Orientation = glm::quat(t.w(),t.x(),t.y(),t.z());

		m_Forward = ObjectDynamic::_Forward();
		m_Right = ObjectDynamic::_Right();
		m_Up = ObjectDynamic::_Up();
	}
	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(parentModel * m_Model);
}
void ObjectDynamic::Set_Position(float x, float y, float z){
	btTransform transform = m_RigidBody->getWorldTransform();
	transform.setOrigin(btVector3(x,y,z));
	m_RigidBody->setWorldTransform(transform);
	Flag_As_Changed();
}
void ObjectDynamic::Set_Position(glm::vec3& p){ ObjectDynamic::Set_Position(p.x,p.y,p.z); }
glm::vec3 ObjectDynamic::Position(){ btVector3 p = m_RigidBody->getWorldTransform().getOrigin(); return glm::vec3(p.x(),p.y(),p.z()); }
void ObjectDynamic::Apply_Force(float x,float y,float z,bool local){ 
	if(!local){
		m_RigidBody->applyCentralForce(btVector3(x,y,z)); 
	}
	else{
		glm::vec3 res = Right() * x;
		res += Up() * y;
		res += Forward() * z;
		m_RigidBody->applyCentralForce(btVector3(res.x,res.y,res.z)); 
	}
	Flag_As_Changed();
}
void ObjectDynamic::Apply_Force(glm::vec3& force,glm::vec3& relPos,bool local){ 
	if(!local){
		m_RigidBody->applyForce(btVector3(force.x,force.y,force.z),btVector3(relPos.x,relPos.y,relPos.z)); 
	}
	else{
		glm::vec3 res = Right() * force.x;
		res += Up() * force.y;
		res += Forward() * force.z;
		m_RigidBody->applyForce(btVector3(res.x,res.y,res.z),btVector3(relPos.x,relPos.y,relPos.z)); 
	}
	Flag_As_Changed();
}
void ObjectDynamic::Apply_Impulse(float x,float y,float z){ 
	m_RigidBody->applyCentralImpulse(btVector3(x,y,z)); 
	Flag_As_Changed();
}
void ObjectDynamic::Apply_Impulse(glm::vec3& impulse,glm::vec3& relPos){ 
	m_RigidBody->applyImpulse(btVector3(impulse.x,impulse.y,impulse.z),btVector3(relPos.x,relPos.y,relPos.z));
	Flag_As_Changed();
}
void ObjectDynamic::Apply_Torque(float x,float y,float z){
	btVector3 t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*btVector3(x,y,z));
	m_RigidBody->applyTorque(t);
	Flag_As_Changed();
}
void ObjectDynamic::Apply_Torque(glm::vec3& torque){ ObjectDynamic::Apply_Torque(torque.x,torque.y,torque.z); }
void ObjectDynamic::Apply_Torque_Impulse(float x,float y,float z){
	btVector3 t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*btVector3(x,y,z));
	m_RigidBody->applyTorqueImpulse(t);
	Flag_As_Changed();
}
void ObjectDynamic::Apply_Torque_Impulse(glm::vec3& torque){ ObjectDynamic::Apply_Torque_Impulse(torque.x,torque.y,torque.z); }
void ObjectDynamic::Set_Linear_Velocity(float x, float y, float z, bool local){
	if(!local){
		m_RigidBody->setLinearVelocity(btVector3(x,y,z)); 
	}
	else{
		glm::vec3 res = Right() * x;
		res += Up() * y;
		res += Forward() * z;
		m_RigidBody->setLinearVelocity(btVector3(res.x,res.y,res.z)); 
	}
	Flag_As_Changed();
}
void ObjectDynamic::Set_Linear_Velocity(glm::vec3& velocity, bool local){ ObjectDynamic::Set_Linear_Velocity(velocity.x,velocity.y,velocity.z,local); }
void ObjectDynamic::Set_Angular_Velocity(float x, float y, float z){ 
	m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
	Flag_As_Changed();
}
void ObjectDynamic::Set_Angular_Velocity(glm::vec3& velocity){ ObjectDynamic::Set_Angular_Velocity(velocity.x,velocity.y,velocity.z); }
void ObjectDynamic::Set_Mass(float mass){
	m_Mass = 0.5f * m_Radius;
	m_Collision_Shape->calculateLocalInertia(m_Mass,*m_Inertia);
	if(m_RigidBody != nullptr)
		m_RigidBody->setMassProps(m_Mass,*m_Inertia);
}
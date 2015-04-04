#include "ObjectDynamic.h"
#include "Engine_Resources.h"

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, std::string name,btCollisionShape* collisionShape): Object(mesh,mat,pos,scl,name,true){
	m_Collision_Shape = collisionShape;
	if(m_Collision_Shape == nullptr){
		if(m_Mesh != nullptr){
			if(m_Mesh->Collision() == nullptr)
				m_Collision_Shape = new btBoxShape(btVector3(m_Radius.x,m_Radius.y,m_Radius.z));
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

	float mass = 0.5f;

	btVector3 inertia;
	m_Collision_Shape->calculateLocalInertia(mass,inertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass,m_MotionState,m_Collision_Shape,inertia);

	m_RigidBody = new btRigidBody(rigidBodyCI);
	m_RigidBody->setFriction(0);
	m_RigidBody->setDamping(0.3f,0.5f);//this makes the objects slowly slow down in space, like air friction
	bullet->Add_Rigid_Body(m_RigidBody);
}
ObjectDynamic::~ObjectDynamic(){
	delete m_Collision_Shape;
	delete m_RigidBody;
	delete m_MotionState;
}
glm::vec3 ObjectDynamic::Forward(){ return glm::normalize(glm::cross(Right(),Up())); }
glm::vec3 ObjectDynamic::Right(){
	btQuaternion q = m_RigidBody->getWorldTransform().getRotation();
	float x = q.x();
	float y = q.y();
	float z = q.z();
	float w = q.w();
	return glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),
                                     2 * (x * y + w * z),
                                     2 * (x * z - w * y)));
}
glm::vec3 ObjectDynamic::Up(){
	btQuaternion q = m_RigidBody->getWorldTransform().getRotation();
	float x = q.x();
	float y = q.y();
	float z = q.z();
	float w = q.w();
	return glm::normalize(glm::vec3( 2 * (x * y - w * z), 
                                     1 - 2 * (x * x + z * z),
                                     2 * (y * z + w * x)));
}
void ObjectDynamic::Update(float dt){
	m_RigidBody->activate();
	m_RigidBody->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m_Model));
	btQuaternion q = m_RigidBody->getWorldTransform().getRotation();
	m_Orientation = glm::quat(q.w(),q.x(),q.y(),q.z());
	glm::mat4 parentModel = glm::mat4(1);
	if(m_Parent != nullptr)
		parentModel = m_Parent->Model();
	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(parentModel * m_Model);
}
void ObjectDynamic::Set_Position(float x, float y, float z){
	btTransform transform = m_RigidBody->getWorldTransform();
	transform.setOrigin(btVector3(x,y,z));
	m_RigidBody->setWorldTransform(transform);
}
void ObjectDynamic::Set_Position(glm::vec3& p){ ObjectDynamic::Set_Position(p.x,p.y,p.z); }
glm::vec3 ObjectDynamic::Position(){
	btVector3 p = m_RigidBody->getWorldTransform().getOrigin();
	return glm::vec3(p.x(),p.y(),p.z());
}
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
}
void ObjectDynamic::Apply_Impulse(float x,float y,float z){ 
	m_RigidBody->applyCentralImpulse(btVector3(x,y,z)); 
}
void ObjectDynamic::Apply_Impulse(glm::vec3& impulse,glm::vec3& relPos){ 
	m_RigidBody->applyImpulse(btVector3(impulse.x,impulse.y,impulse.z),btVector3(relPos.x,relPos.y,relPos.z)); 
}
void ObjectDynamic::Apply_Torque(float x,float y,float z){
	btVector3 t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*btVector3(x,y,z));
	m_RigidBody->applyTorque(t);
}
void ObjectDynamic::Apply_Torque(glm::vec3& torque){ ObjectDynamic::Apply_Torque(torque.x,torque.y,torque.z); }
void ObjectDynamic::Apply_Torque_Impulse(float x,float y,float z){
	btVector3 t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*btVector3(x,y,z));
	m_RigidBody->applyTorqueImpulse(t);
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
}
void ObjectDynamic::Set_Linear_Velocity(glm::vec3& velocity, bool local){ ObjectDynamic::Set_Linear_Velocity(velocity.x,velocity.y,velocity.z,local); }
void ObjectDynamic::Set_Angular_Velocity(float x, float y, float z){ 
	m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::Set_Angular_Velocity(glm::vec3& velocity){ ObjectDynamic::Set_Angular_Velocity(velocity.x,velocity.y,velocity.z); }
/*
void ObjectDynamic::Apply_Rotation_Force(float x, float y, float z, bool local){
	x *= Resources->dt; y *= Resources->dt; z *= Resources->dt;
	m_RotationalForce += glm::vec3(x,y,z);
}
void ObjectDynamic::Apply_Rotation_Force(glm::vec3& force, bool local){ ObjectDynamic::Apply_Rotation_Force(force.x,force.y,force.z,local); }
*/
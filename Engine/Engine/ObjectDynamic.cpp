#include "ObjectDynamic.h"
#include "Engine_Resources.h"

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, std::string name,btCollisionShape* collisionShape): Object(mesh,mat,pos,scl,name,true){
	m_Collision_Shape = collisionShape;
	if(m_Collision_Shape == nullptr){
		m_Collision_Shape = m_Mesh->Collision();
		//m_Collision_Shape = new btBoxShape(btVector3(m_Radius.x,m_Radius.y,m_Radius.z));
	}

	btTransform tr;
	glm::mat4 m = glm::mat4(1);
	m = glm::translate(m,m_Position);
	m *= glm::mat4_cast(m_Orientation);
	m = glm::scale(m,m_Scale);
	tr.setFromOpenGLMatrix(glm::value_ptr(m));

	m_MotionState = new btDefaultMotionState(tr);

	//													 mass
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.5f,m_MotionState,m_Collision_Shape);

	m_RigidBody = new btRigidBody(rigidBodyCI);
	m_RigidBody->setDamping(0.3f,0.3f);//this makes the objects slowly slow down in space, like air friction
	bullet->Add_Rigid_Body(m_RigidBody);
}
ObjectDynamic::~ObjectDynamic(){
	delete m_Collision_Shape;
	delete m_RigidBody;
	delete m_MotionState;
}
void ObjectDynamic::Update(float dt){
	m_RigidBody->activate();
	m_RigidBody->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m_Model));

	glm::mat4 parentModel = glm::mat4(1);
	if(m_Parent != nullptr)
		parentModel = m_Parent->Model();
	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(parentModel * m_Model);
}

void ObjectDynamic::Set_Position(float x, float y, float z){
	m_RigidBody->activate();
	btTransform transform = m_RigidBody->getWorldTransform();
	transform.setOrigin(btVector3(x,y,z));
	m_RigidBody->setWorldTransform(transform);
}
void ObjectDynamic::Set_Position(glm::vec3& p){ ObjectDynamic::Set_Position(p.x,p.y,p.z); }
void ObjectDynamic::Translate(float x,float y,float z){
	m_RigidBody->activate();

	glm::vec3 pos = glm::vec3(0,0,0);
	pos += Forward() * z * Resources->dt;
	pos += Right() * x * Resources->dt;
	pos += Up() * y * Resources->dt;

	m_RigidBody->translate(btVector3(pos.x,pos.y,pos.z));
}
void ObjectDynamic::Translate(glm::vec3& t){ ObjectDynamic::Translate(t.x,t.y,t.z); }
void ObjectDynamic::Rotate(float x, float y, float z){
	m_RigidBody->activate();

	Pitch(x);
	Yaw(y);
	Roll(z);

	btQuaternion q = btQuaternion(m_Orientation.x,m_Orientation.y,m_Orientation.z,m_Orientation.w);
	m_RigidBody->getWorldTransform().setRotation(q);
}
void ObjectDynamic::Rotate(glm::vec3& r){ 
	ObjectDynamic::Rotate(r.x,r.y,r.z); 
}
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
	m_RigidBody->applyTorque(btVector3(x,y,z)); 
}
void ObjectDynamic::Apply_Torque(glm::vec3& torque){ 
	m_RigidBody->applyTorque(btVector3(torque.x,torque.y,torque.z)); 
}
void ObjectDynamic::Apply_Torque_Impulse(float x,float y,float z){ 
	m_RigidBody->applyTorqueImpulse(btVector3(x,y,z)); 
}
void ObjectDynamic::Apply_Torque_Impulse(glm::vec3& torque){ 
	m_RigidBody->applyTorqueImpulse(btVector3(torque.x,torque.y,torque.z)); 
}
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
void ObjectDynamic::Set_Linear_Velocity(glm::vec3& velocity, bool local){ 
	ObjectDynamic::Set_Linear_Velocity(velocity.x,velocity.y,velocity.z,local); 
}
void ObjectDynamic::Set_Angular_Velocity(float x, float y, float z){ 
	m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::Set_Angular_Velocity(glm::vec3& velocity){ 
	m_RigidBody->setAngularVelocity(btVector3(velocity.x,velocity.y,velocity.z)); 
}
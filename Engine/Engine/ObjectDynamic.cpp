#include "ObjectDynamic.h"
#include "Engine_Resources.h"

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, glm::vec3 rot, std::string name,btCollisionShape* collisionShape): Object(mesh,mat,pos,scl,rot,name,true){
	m_Collision_Shape = collisionShape;
	if(m_Collision_Shape == nullptr){
		//m_Collision_Shape = m_Mesh->Collision();
		m_Collision_Shape = new btBoxShape(btVector3(m_Radius.x,m_Radius.y,m_Radius.z));
	}

	m_MotionState = new btDefaultMotionState(btTransform(
		btQuaternion(),
		btVector3(m_Position.x,m_Position.y,m_Position.z)
	));

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
		0.5f,                  // mass, in kg. 0 -> Static object, will never move.
		m_MotionState,
		m_Collision_Shape,
		btVector3(0,0,0)    // inertia
	);
	m_RigidBody = new btRigidBody(rigidBodyCI);

	bullet->Add_Rigid_Body(m_RigidBody);

	ObjectDynamic::Set_Position(pos);
}
ObjectDynamic::~ObjectDynamic(){
	delete m_Collision_Shape;
	delete m_RigidBody;
	delete m_MotionState;
}
void ObjectDynamic::Update(float dt){
	m_Model = glm::mat4(1);
	if(m_Parent != nullptr)
		m_Model = m_Parent->Model();

	btTransform objTrans = m_RigidBody->getCenterOfMassTransform();
	objTrans.getOpenGLMatrix(glm::value_ptr(m_Model));

	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(m_Model);
}
void ObjectDynamic::Render(Mesh* mesh, Material* material,RENDER_TYPE rType){
	Object::Render(mesh,material,rType);
}
glm::vec3 ObjectDynamic::Forward(btQuaternion& orientation) const{
	float x = orientation.getX();
	float y = orientation.getY();
	float z = orientation.getZ();
	float w = orientation.getW();
	return glm::vec3( 2 * (x * z + w * y), 
                      2 * (y * x - w * x),
                      1 - 2 * (x * x + y * y));
}
glm::vec3 ObjectDynamic::Right(btQuaternion& orientation) const{
	float x = orientation.getX();
	float y = orientation.getY();
	float z = orientation.getZ();
	float w = orientation.getW();
    return glm::vec3( 1 - 2 * (y * y + z * z),
                      2 * (x * y + w * z),
                      2 * (x * z - w * y));
}
glm::vec3 ObjectDynamic::Up(btQuaternion& orientation) const{
	float x = orientation.getX();
	float y = orientation.getY();
	float z = orientation.getZ();
	float w = orientation.getW();
    return glm::vec3( 2 * (x * y - w * z), 
                      1 - 2 * (x * x + z * z),
                      2 * (y * z + w * x));
}

void ObjectDynamic::Set_Position(float x, float y, float z){
	m_RigidBody->activate();
	btTransform transform = m_RigidBody->getCenterOfMassTransform();
	transform.setOrigin(btVector3(x,y,z));
	m_RigidBody->setCenterOfMassTransform(transform);
}
void ObjectDynamic::Set_Position(glm::vec3 p){ ObjectDynamic::Set_Position(p.x,p.y,p.z); }
void ObjectDynamic::Translate(float x,float y,float z){
	m_RigidBody->activate();
	x *= Resources->dt; y *= Resources->dt; z *= Resources->dt;
	m_RigidBody->translate(btVector3(x,y,z));
}
void ObjectDynamic::Translate(glm::vec3 t){ ObjectDynamic::Translate(t.x,t.y,t.z); }
void ObjectDynamic::Rotate(float x, float y, float z){
	m_RigidBody->activate();
	ObjectDynamic::Pitch(x);
	ObjectDynamic::Yaw(y);
	ObjectDynamic::Roll(z);

	btTransform tr;
	tr.setFromOpenGLMatrix(glm::value_ptr(glm::lookAt(m_Position,m_Forward,m_Up)));

	m_RigidBody->setCenterOfMassTransform(tr);
}
void ObjectDynamic::Rotate(glm::vec3 r){
	ObjectDynamic::Rotate(r.x,r.y,r.z);
}
void ObjectDynamic::Pitch(float amount){
	glm::quat rotationQuaternion = glm::quat(-amount * Resources->dt,m_Right);
	rotationQuaternion = glm::conjugate(rotationQuaternion);
	glm::vec4 result =  glm::mat4_cast(rotationQuaternion) * glm::vec4(m_Up,1);
	m_Up = glm::normalize(glm::vec3(-result.x,-result.y,-result.z));
	m_Forward = glm::normalize(glm::cross(m_Up,m_Right));
}
void ObjectDynamic::Yaw(float amount){
	glm::quat rotationQuaternion = glm::quat(-amount * Resources->dt,m_Up);
	rotationQuaternion = glm::conjugate(rotationQuaternion);
	glm::vec4 result = glm::mat4_cast(rotationQuaternion) * glm::vec4(m_Right,1);
	m_Right = glm::normalize(glm::vec3(-result.x,-result.y,-result.z));
	m_Forward = glm::normalize(glm::cross(m_Up,m_Right));
}
void ObjectDynamic::Roll(float amount){
	glm::quat rotationQuaternion = glm::quat(-amount * Resources->dt,m_Forward);
	rotationQuaternion = glm::conjugate(rotationQuaternion);
	glm::vec4 result = glm::mat4_cast(rotationQuaternion) * glm::vec4(m_Up,1);
	m_Up = glm::normalize(glm::vec3(-result.x,-result.y,-result.z));
	m_Right = glm::normalize(-glm::cross(m_Up,m_Forward));
}
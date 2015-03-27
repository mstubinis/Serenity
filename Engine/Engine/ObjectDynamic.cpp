#include "ObjectDynamic.h"
#include "Engine_Resources.h"

ObjectDynamic::ObjectDynamic(Mesh* mesh, Material* material, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, std::string name,btCollisionShape* collisionShape): Object(mesh,material,position,scale,rotation,name,true){
	m_Velocity = m_Velocity_Rotation = glm::vec3(0,0,0);
	m_Friction = 1;
	m_Mass = 0;
	m_Inertia = 0;

	m_Collision_Shape = collisionShape;
	if(m_Collision_Shape == nullptr)
		m_Collision_Shape = new btBoxShape(btVector3(this->m_Radius.x, this->m_Radius.y, this->m_Radius.z));

	m_MotionState = new btDefaultMotionState(btTransform(
		btQuaternion(0,0,0,1), 
		btVector3(0,0,0)
	));

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
		0.5f,                  // mass, in kg. 0 -> Static object, will never move.
		m_MotionState,
		m_Collision_Shape,
		btVector3(0,0,0)    // inertia
	);
	m_RigidBody = new btRigidBody(rigidBodyCI);
	m_RigidBody->activate();
	m_RigidBody->setFriction(m_Friction);


	bullet->Add_Rigid_Body(m_RigidBody);
}
ObjectDynamic::~ObjectDynamic()
{
}
void ObjectDynamic::Update(float dt)
{
	Object::Update(dt);
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
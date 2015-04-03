#include "ObjectDynamic.h"
#include "Engine_Resources.h"


btQuaternion LookAt(glm::vec3 forward, glm::vec3 up){
	forward = glm::normalize(forward);
 
    glm::vec3 vector = forward;
	glm::vec3 vector2 = glm::normalize(glm::cross(up,vector));
	glm::vec3 vector3 = glm::normalize(glm::cross(vector,vector2));
    float m00 = vector2.x;
    float m01 = vector2.y;
    float m02 = vector2.z;
    float m10 = vector3.x;
    float m11 = vector3.y;
    float m12 = vector3.z;
    float m20 = vector.x;
    float m21 = vector.y;
    float m22 = vector.z;
 
    float num8 = (m00 + m11) + m22;
    btQuaternion quaternion = btQuaternion();
    if (num8 > 0.0f){
        float num = static_cast<float>(sqrt(num8 + 1.0f));
        quaternion.setW(num * 0.5f);
        num = 0.5f / num;
        quaternion.setX((m12 - m21) * num);
        quaternion.setY((m20 - m02) * num);
        quaternion.setZ((m01 - m10) * num);
        return quaternion;
    }
    if ((m00 >= m11) && (m00 >= m22)){
        float num7 = static_cast<float>(sqrt(((1.0 + m00) - m11) - m22));
        float num4 = 0.5f / num7;
        quaternion.setX(0.5f * num7);
        quaternion.setY((m01 + m10) * num4);
        quaternion.setZ((m02 + m20) * num4);
        quaternion.setW((m12 - m21) * num4);
        return quaternion;
    }
    if (m11 > m22){
        float num6 = static_cast<float>(sqrt(((1.0f + m11) - m00) - m22));
        float num3 = 0.5f / num6;
        quaternion.setX((m10 + m01) * num3);
        quaternion.setY(0.5f * num6);
        quaternion.setZ((m21 + m12) * num3);
        quaternion.setW((m20 - m02) * num3);
        return quaternion;
    }
    float num5 = static_cast<float>(sqrt(((1.0f + m22) - m00) - m11));
    float num2 = 0.5f / num5;
    quaternion.setX((m20 + m02) * num2);
    quaternion.setY((m21 + m12) * num2);
    quaternion.setZ(0.5f * num5);
    quaternion.setW((m01 - m10) * num2);
    return quaternion;
}

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl, glm::vec3 rot, std::string name,btCollisionShape* collisionShape): Object(mesh,mat,pos,scl,rot,name,true){
	m_Collision_Shape = collisionShape;
	m_Rotation = glm::vec3(0,0,0);
	if(m_Collision_Shape == nullptr){
		m_Collision_Shape = m_Mesh->Collision();
		//m_Collision_Shape = new btBoxShape(btVector3(m_Radius.x,m_Radius.y,m_Radius.z));
	}

	btTransform tr;
	glm::mat4 m = glm::mat4(1);
	m = glm::translate(m,m_Position);
	m *= glm::transpose(glm::lookAt(glm::vec3(0,0,0),m_Forward,m_Up));
	tr.setFromOpenGLMatrix(glm::value_ptr(m));

	m_MotionState = new btDefaultMotionState(tr);

	//													 mass
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.5f,m_MotionState,m_Collision_Shape);

	m_RigidBody = new btRigidBody(rigidBodyCI);
	m_RigidBody->setDamping(0.3f,0.3f);//this makes the objects slowly slow down in space, like air friction
	bullet->Add_Rigid_Body(m_RigidBody);

	ObjectDynamic::Set_Position(pos);
}
ObjectDynamic::~ObjectDynamic(){
	delete m_Collision_Shape;
	delete m_RigidBody;
	delete m_MotionState;
}
void ObjectDynamic::Update(float dt){
	m_RigidBody->activate();

	m_RigidBody->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m_Model));
	glm::mat4 newModel = glm::mat4(1);
	if(m_Parent != nullptr)
		newModel = m_Parent->Model();
	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(newModel * m_Model);
}
void ObjectDynamic::Render(Mesh* mesh, Material* material,RENDER_TYPE rType){
	Object::Render(mesh,material,rType);
}
glm::vec3 ObjectDynamic::Forward(btQuaternion& orientation) const{
	float x = orientation.getX();
	float y = orientation.getY();
	float z = orientation.getZ();
	float w = orientation.getW();
	return glm::normalize(-glm::vec3( 2 * (x * z + w * y), 
                                      2 * (y * x - w * x),
                                      1 - 2 * (x * x + y * y)));
}
glm::vec3 ObjectDynamic::Right(btQuaternion& orientation) const{
	float x = orientation.getX();
	float y = orientation.getY();
	float z = orientation.getZ();
	float w = orientation.getW();
	return glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),
                                     2 * (x * y + w * z),
                                     2 * (x * z - w * y)));
}
glm::vec3 ObjectDynamic::Up(btQuaternion& orientation) const{
	float x = orientation.getX();
	float y = orientation.getY();
	float z = orientation.getZ();
	float w = orientation.getW();
	return glm::normalize(glm::vec3( 2 * (x * y - w * z), 
                                     1 - 2 * (x * x + z * z),
                                     2 * (y * z + w * x)));
}

void ObjectDynamic::Set_Position(float x, float y, float z){
	m_RigidBody->activate();
	btTransform transform = m_RigidBody->getCenterOfMassTransform();
	transform.setOrigin(btVector3(x,y,z));
	m_RigidBody->setCenterOfMassTransform(transform);
}
void ObjectDynamic::Set_Position(glm::vec3& p){ ObjectDynamic::Set_Position(p.x,p.y,p.z); }
void ObjectDynamic::Translate(float x,float y,float z){
	m_RigidBody->activate();

	glm::vec3 pos = glm::normalize(m_Forward) * z * Resources->dt;
	pos += glm::normalize(m_Right) * x * Resources->dt;
	pos += glm::normalize(m_Up) * y * Resources->dt;

	m_RigidBody->translate(btVector3(pos.x,pos.y,pos.z));
}
void ObjectDynamic::Translate(glm::vec3& t){ ObjectDynamic::Translate(t.x,t.y,t.z); }
void ObjectDynamic::Rotate(float x, float y, float z){
	m_RigidBody->activate();

	Pitch(x);
	Yaw(y);
	Roll(z);

	btTransform t = m_RigidBody->getCenterOfMassTransform();
	btVector3 p = t.getOrigin();
	glm::mat4 matrix = glm::mat4(1);
	matrix = glm::translate(matrix,glm::vec3(p.x(),p.y(),p.z()));
	matrix *= glm::transpose(glm::lookAt(glm::vec3(0,0,0),m_Forward,m_Up));
	t.setFromOpenGLMatrix(glm::value_ptr(matrix));

	m_RigidBody->setCenterOfMassTransform(t);
}
void ObjectDynamic::Rotate(glm::vec3& r){ ObjectDynamic::Rotate(r.x,r.y,r.z); }
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
glm::vec3 ObjectDynamic::Position(){
	btVector3 p = m_RigidBody->getCenterOfMassTransform().getOrigin();
	return glm::vec3(p.x(),p.y(),p.z());
}
glm::vec3 ObjectDynamic::Forward(){
	btQuaternion q = m_RigidBody->getCenterOfMassTransform().getRotation();
	return Forward(q);
}
glm::vec3 ObjectDynamic::Right(){
	btQuaternion q = m_RigidBody->getCenterOfMassTransform().getRotation();
	return Right(q);
}
glm::vec3 ObjectDynamic::Up(){
	btQuaternion q = m_RigidBody->getCenterOfMassTransform().getRotation();
	return Up(q);
}
void ObjectDynamic::Apply_Force(float x,float y,float z,bool local){ 
	if(!local){
		m_RigidBody->applyCentralForce(btVector3(x,y,z)); 
	}
	else{
		glm::vec3 res = m_Right * x;
		res += m_Up * y;
		res += m_Forward * z;
		m_RigidBody->applyCentralForce(btVector3(res.x,res.y,res.z)); 
	}
}
void ObjectDynamic::Apply_Force(glm::vec3& force,glm::vec3& relPos,bool local){ 
	if(!local){
		m_RigidBody->applyForce(btVector3(force.x,force.y,force.z),btVector3(relPos.x,relPos.y,relPos.z)); 
	}
	else{
		glm::vec3 res = m_Right * force.x;
		res += m_Up * force.y;
		res += m_Forward * force.z;
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
		glm::vec3 res = m_Right * x;
		res += m_Up * y;
		res += m_Forward * z;
		m_RigidBody->setLinearVelocity(btVector3(res.x,res.y,res.z)); 
	}
}
void ObjectDynamic::Set_Linear_Velocity(glm::vec3& velocity, bool local){ ObjectDynamic::Set_Linear_Velocity(velocity.x,velocity.y,velocity.z,local); }
void ObjectDynamic::Set_Angular_Velocity(float x, float y, float z){ 
	m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::Set_Angular_Velocity(glm::vec3& velocity){ 
	m_RigidBody->setAngularVelocity(btVector3(velocity.x,velocity.y,velocity.z)); 
}


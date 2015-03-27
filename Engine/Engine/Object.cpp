#include "Object.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"

Object::Object(Mesh* mesh, Material* material, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation,std::string name,bool addToResources){
	m_Name = name;
	m_Position = position;
	m_Scale = scale;
	m_Rotation = rotation;
	m_Model = m_WorldMatrix = glm::mat4(1);
	m_Forward = glm::vec3(0,0,-1);
	m_Right = glm::vec3(1,0,0);
	m_Up = glm::vec3(0,1,0);
	m_Color = glm::vec3(1,1,1);
	m_Parent = nullptr;

	Set_Mesh(mesh);
	Set_Material(material);

	if(addToResources == true)
		Resources->Objects.push_back(this);
}
Object::~Object()
{
}
void Object::m_Calculate_Radius(){
	m_Radius *= m_Scale;
}
void Object::Translate(float x, float y, float z){ 
	m_Position += glm::normalize(m_Forward) * (z * Resources->dt);
	m_Position += glm::normalize(m_Right) * (x * Resources->dt);
	m_Position += glm::normalize(m_Up) * (y * Resources->dt);
}
void Object::Translate(glm::vec3& translation){ Translate(translation.x,translation.y,translation.z); }
void Object::Rotate(float x, float y, float z){ Pitch(x); Yaw(y); Roll(z); }
void Object::Rotate(glm::vec3& rotation){ Rotate(rotation.x,rotation.y,rotation.z); }
void Object::Scale(float x, float y, float z){ m_Scale.x += x * Resources->dt; m_Scale.y += y * Resources->dt; m_Scale.z += z * Resources->dt; m_Calculate_Radius(); }
void Object::Scale(glm::vec3& scale){ Scale(scale.x,scale.y,scale.z); }
void Object::Set_Position(float x, float y, float z){ m_Position.x = x; m_Position.y = y; m_Position.z = z; }
void Object::Set_Position(glm::vec3& position){ Set_Position(position.x,position.y,position.z); }
void Object::Set_Rotation(float x, float y, float z){ m_Rotation.x = x; m_Rotation.y = y; m_Rotation.z = z; }
void Object::Set_Rotation(glm::vec3& rotation){ Set_Rotation(rotation.x,rotation.y,rotation.z); }
void Object::Set_Scale(float x, float y, float z){ m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; m_Calculate_Radius(); }
void Object::Set_Scale(glm::vec3& scale){ Set_Scale(scale.x,scale.y,scale.z); }
void Object::Pitch(float amount){
	glm::quat rotationQuaternion = glm::quat(-amount * Resources->dt,m_Right);
	rotationQuaternion = glm::conjugate(rotationQuaternion);
	glm::vec4 result =  glm::mat4_cast(rotationQuaternion) * glm::vec4(m_Up,1);
	m_Up = glm::normalize(glm::vec3(-result.x,-result.y,-result.z));
	m_Forward = glm::normalize(glm::cross(m_Up,m_Right));
}
void Object::Yaw(float amount){
	glm::quat rotationQuaternion = glm::quat(-amount * Resources->dt,m_Up);
	rotationQuaternion = glm::conjugate(rotationQuaternion);
	glm::vec4 result = glm::mat4_cast(rotationQuaternion) * glm::vec4(m_Right,1);
	m_Right = glm::normalize(glm::vec3(-result.x,-result.y,-result.z));
	m_Forward = glm::normalize(glm::cross(m_Up,m_Right));
}
void Object::Roll(float amount){
	glm::quat rotationQuaternion = glm::quat(-amount * Resources->dt,m_Forward);
	rotationQuaternion = glm::conjugate(rotationQuaternion);
	glm::vec4 result = glm::mat4_cast(rotationQuaternion) * glm::vec4(m_Up,1);
	m_Up = glm::normalize(glm::vec3(-result.x,-result.y,-result.z));
	m_Right = glm::normalize(-glm::cross(m_Up,m_Forward));
}
void Object::Update(float dt){
	m_Model = glm::mat4(1);
	if(m_Parent != nullptr)
		m_Model = m_Parent->m_Model;

	m_Model = glm::translate(m_Model, m_Position);
	m_Model *= glm::transpose(glm::lookAt(glm::vec3(0,0,0),m_Forward,m_Up));
	m_Model = glm::scale(m_Model,m_Scale);

	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(m_Model);
}
void Object::Render(Mesh* mesh, Material* material,RENDER_TYPE renderType){
	if(mesh == nullptr)
		return;

	GLuint shaderProgram;
	if(renderType == RENDER_TYPE_FORWARD)
		shaderProgram = Resources->Get_Shader_Program("Default")->Get_Shader_Program();
	else if(renderType == RENDER_TYPE_DEFERRED)
		shaderProgram = Resources->Get_Shader_Program("Deferred")->Get_Shader_Program();

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform3f(glGetUniformLocation(shaderProgram, "Object_Color"),m_Color.x,m_Color.y,m_Color.z);

	for(auto component:material->Components())
		material->Bind_Texture(component.second);

	mesh->Render();
}
void Object::Render(RENDER_TYPE rType){ Object::Render(m_Mesh,m_Material,rType); }
void Object::Add_Child(Object* child){
	child->m_Parent = this;
	this->m_Children.push_back(child);
}
#pragma region Getters
glm::vec3 Object::Position() { return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
const glm::vec3& Object::Rotation() const { return m_Rotation; }
const glm::vec3& Object::Scale() const { return m_Scale; }
const glm::vec3& Object::Forward() const { return m_Forward; }
const glm::vec3& Object::Right() const { return m_Right; }
const glm::vec3& Object::Up() const { return m_Up; }
const glm::vec3& Object::Radius() const { return m_Radius; }
const glm::vec3& Object::Color() const { return m_Color; }
const glm::mat4& Object::Model() const { return m_Model; }
const glm::mat4& Object::World() const { return m_WorldMatrix; }
std::string Object::Name() const { return m_Name; }
const Object* Object::Parent() const { return m_Parent; }
std::vector<Object*> Object::Children() const { return m_Children; }
#pragma endregion
#pragma region Setters
void Object::Set_Mesh(Mesh* mesh){ m_Mesh = mesh; if(m_Mesh == nullptr){m_Radius = glm::vec3(0,0,0); return; } m_Radius = mesh->Radius();  m_Calculate_Radius();  }
void Object::Set_Material(Material* material){ m_Material = material; }
#pragma endregion
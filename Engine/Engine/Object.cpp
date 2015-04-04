#include "Object.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"

Object::Object(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl,std::string name,bool addToResources){
	Set_Mesh(Resources->Get_Mesh(mesh));
	Set_Material(Resources->Get_Material(mat));
	m_Parent = nullptr;

	m_Name = name;
	m_Model = m_WorldMatrix = glm::mat4(1);
	m_Orientation = glm::quat();
	m_Color = glm::vec3(1,1,1);

	Object::Set_Position(pos);
	Object::Set_Scale(scl);

	if(addToResources == true)
		Resources->Objects.push_back(this);
}
Object::~Object()
{
}
void Object::m_Calculate_Radius(){
	if(m_Mesh == nullptr){
		m_Radius = glm::vec3(0,0,0);
		return;
	}
	m_Radius = m_Mesh->Radius() * m_Scale;
}
glm::vec3 Object::Forward(){ return glm::normalize(glm::cross(Right(),Up())); }
glm::vec3 Object::Right(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	return glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),
                                     2 * (x * y + w * z),
                                     2 * (x * z - w * y)));
}
glm::vec3 Object::Up(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	return glm::normalize(glm::vec3( 2 * (x * y - w * z), 
                                     1 - 2 * (x * x + z * z),
                                     2 * (y * z + w * x)));
}
void Object::Translate(float x, float y, float z){
	m_Position += Forward() * (z * Resources->dt);
	m_Position += Right() * (x * Resources->dt);
	m_Position += Up() * (y * Resources->dt);
}
void Object::Translate(glm::vec3& translation){ Translate(translation.x,translation.y,translation.z); }
void Object::Rotate(float x, float y, float z){ 
	float threshold = 0.025f;
	if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
		return;
	Pitch(x); 
	Yaw(y); 
	Roll(z); 
}
void Object::Rotate(glm::vec3& rotation){ Rotate(rotation.x,rotation.y,rotation.z); }
void Object::Scale(float x, float y, float z){ m_Scale.x += x * Resources->dt; m_Scale.y += y * Resources->dt; m_Scale.z += z * Resources->dt; m_Calculate_Radius(); }
void Object::Scale(glm::vec3& scale){ Scale(scale.x,scale.y,scale.z); }
void Object::Set_Position(float x, float y, float z){ m_Position.x = x; m_Position.y = y; m_Position.z = z; }
void Object::Set_Position(glm::vec3& position){ Set_Position(position.x,position.y,position.z); }
void Object::Set_Scale(float x, float y, float z){ m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; m_Calculate_Radius(); }
void Object::Set_Scale(glm::vec3& scale){ Set_Scale(scale.x,scale.y,scale.z); }
void Object::Pitch(float amount){ m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(1,0,0))); }
void Object::Yaw(float amount){ m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(0,1,0))); }
void Object::Roll(float amount){ m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(amount,glm::vec3(0,0,1))); }
void Object::Update(float dt){
	glm::mat4 newModel = glm::mat4(1);
	if(m_Parent != nullptr){
		newModel = m_Parent->m_Model;
		m_Orientation = m_Parent->Orientation();
	}

	newModel = glm::translate(newModel, m_Position);
	newModel *= glm::mat4_cast(m_Orientation);
	newModel = glm::scale(newModel,m_Scale);
	m_Model = newModel;

	m_WorldMatrix = Resources->Current_Camera()->Calculate_Projection(m_Model);
}
void Object::Render(Mesh* mesh, Material* material,bool debug){
	if(mesh == nullptr)
		return;

	GLuint shaderProgram = Resources->Get_Shader_Program("Deferred")->Get_Shader_Program();

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform3f(glGetUniformLocation(shaderProgram, "Object_Color"),m_Color.x,m_Color.y,m_Color.z);
	glUniform1i(glGetUniformLocation(shaderProgram, "Shadeless"),static_cast<int>(material->Shadeless()));
	for(auto component:material->Components())
		material->Bind_Texture(component.second,shaderProgram);

	mesh->Render();

	if(debug){
	}
}
void Object::Render(bool debug){ Object::Render(m_Mesh,m_Material,debug); }
void Object::Add_Child(Object* child){
	child->m_Parent = this;
	this->m_Children.push_back(child);
}
void Object::Set_Color(float x, float y, float z){ m_Color.x = x; m_Color.y = y; m_Color.z = z; }
void Object::Set_Color(glm::vec3& color){ Object::Set_Color(color.x,color.y,color.z); }
#pragma region Getters
glm::vec3 Object::Position() const { return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
glm::vec3 Object::Scale() { return m_Scale; }
const glm::vec3& Object::Radius() const { return m_Radius; }
const glm::vec3& Object::Color() const { return m_Color; }
const glm::mat4& Object::Model() const { return m_Model; }
const glm::mat4& Object::World() const { return m_WorldMatrix; }
const std::string Object::Name() const { return m_Name; }
const Object* Object::Parent() const { return m_Parent; }
std::vector<Object*> Object::Children() const { return m_Children; }
#pragma endregion
#pragma region Setters
void Object::Set_Mesh(Mesh* mesh){ m_Mesh = mesh; if(m_Mesh == nullptr){m_Radius = glm::vec3(0,0,0); return; } m_Radius = mesh->Radius();  m_Calculate_Radius();  }
void Object::Set_Material(Material* material){ m_Material = material; }
#pragma endregion
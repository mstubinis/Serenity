#include "Object.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

Object::Object(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl,std::string name,bool isNotLight,Scene* scene){
	m_Changed = true;
	m_Radius = 0;
	m_BoundingBoxRadius = glm::vec3(0,0,0);
	m_Forward = glm::vec3(0,0,-1);
	m_Right = glm::vec3(1,0,0);
	m_Up = glm::vec3(0,1,0);
	setMesh(Resources::getMesh(mesh));
	setMaterial(Resources::getMaterial(mat));
	m_Parent = nullptr;

	m_Name = name;
	m_Model = m_WorldMatrix = glm::mat4(1);
	m_Orientation = glm::quat();
	m_Color = glm::vec3(1,1,1);

	Object::setPosition(pos);
	Object::setScale(scl);

	unsigned int count = 0;
	if(scene == nullptr){
		scene = Resources::getCurrentScene();
	}
	if (scene->getObjects().size() > 0){
		while(scene->getObjects().count(m_Name)){
			m_Name = name + " " + boost::lexical_cast<std::string>(count);
			count++;
		}
	}
	if(isNotLight)
		scene->getObjects()[m_Name] = this;
}
Object::~Object()
{
}
void Object::calculateRadius(){
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0,0,0);
		return;
	}
	m_BoundingBoxRadius = m_Mesh->getRadius() * m_Scale;
	m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
}
glm::vec3 Object::_calculateForward(){ return glm::normalize(glm::cross(getRight(),getUp())); }
glm::vec3 Object::_calculateRight(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	return glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),
                                     2 * (x * y + w * z),
                                     2 * (x * z - w * y)));
}
glm::vec3 Object::_calculateUp(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	return glm::normalize(glm::vec3( 2 * (x * y - w * z), 
                                     1 - 2 * (x * x + z * z),
                                     2 * (y * z + w * x)));
}
void Object::translate(float x, float y, float z,bool local){
	float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
	x *= dt; y*= dt; z *= dt;
	if(local){
		m_Position += getForward() * z;
		m_Position += getRight() * x;
		m_Position += getUp() * y;
	}
	else{
		m_Position += glm::vec3(x,y,z);
	}
	flagAsChanged();
}
void Object::translate(glm::vec3& translation,bool local){ translate(translation.x,translation.y,translation.z); }
void Object::rotate(float x, float y, float z){ 
	float threshold = 0.025f;
	if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
		return;
	pitch(x); 
	yaw(y); 
	roll(z); 
	m_Changed = true;
}
void Object::rotate(glm::vec3& rotation){ rotate(rotation.x,rotation.y,rotation.z); }
void Object::scale(float x, float y, float z){
	float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
	m_Scale.x += x * dt; 
	m_Scale.y += y * dt; 
	m_Scale.z += z * dt; 
	calculateRadius(); 
	flagAsChanged();
}
void Object::scale(glm::vec3& scl){ scale(scl.x,scl.y,scl.z); }
void Object::setPosition(float x, float y, float z){ 
	m_Position.x = x; 
	m_Position.y = y; 
	m_Position.z = z; 
	flagAsChanged();
}
void Object::setPosition(glm::vec3 position){ setPosition(position.x,position.y,position.z); }
void Object::setScale(float x, float y, float z){ 
	m_Scale.x = x; 
	m_Scale.y = y; 
	m_Scale.z = z; 
	calculateRadius(); 
	flagAsChanged();
}
void Object::setScale(glm::vec3& scale){ setScale(scale.x,scale.y,scale.z); }
void Object::pitch(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(1,0,0)));
	flagAsChanged();
}
void Object::yaw(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(0,1,0)));
	flagAsChanged();
}
void Object::roll(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(amount,glm::vec3(0,0,1)));
	flagAsChanged();
}
void Object::update(float dt){
	glm::mat4 newModel = glm::mat4(1);
	if(m_Parent != nullptr){
		newModel = m_Parent->m_Model;
	}
	if(m_Changed){
		m_Forward = _calculateForward();
		m_Right = _calculateRight();
		m_Up = _calculateUp();

		newModel = glm::translate(newModel, m_Position);
		newModel *= glm::mat4_cast(m_Orientation);
		newModel = glm::scale(newModel,m_Scale);
		m_Model = newModel;
		m_Changed = false;
	}
	m_WorldMatrix = Resources::getActiveCamera()->calculateProjection(m_Model);
}
float Object::getDistance(Object* other){
	glm::vec3 vecTo = other->getPosition() - getPosition();
	return abs(glm::length(vecTo));
}
void Object::render(Mesh* mesh, Material* material,bool debug){
	if(mesh == nullptr)
		return;
	if(!Resources::getActiveCamera()->sphereIntersectTest(this))
		return;
	if(Resources::getActiveCamera()->getDistance(this) > 450 * getRadius())
		return;

	GLuint shader = Resources::getShader("Deferred")->getShaderProgram();

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform3f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z);
	glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(material->getShadeless()));

	glUniform1f(glGetUniformLocation(shader, "far"),Resources::getActiveCamera()->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	for(auto component:material->getComponents())
		material->bindTexture(component.first,shader);
	mesh->render();
}
void Object::render(bool debug){ Object::render(m_Mesh,m_Material,debug); }
void Object::addChild(Object* child){
	child->m_Parent = this;
	m_Children.push_back(child);
	child->flagAsChanged();
	flagAsChanged();
}
void Object::setColor(float x, float y, float z){ m_Color.x = x; m_Color.y = y; m_Color.z = z; }
void Object::setColor(glm::vec3& color){ Object::setColor(color.x,color.y,color.z); }

void Object::setMesh(Mesh* mesh){ 
	m_Mesh = mesh; 
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0,0,0); 
		return; 
	} 
	m_BoundingBoxRadius = mesh->getRadius();  
	calculateRadius();
}
void Object::setMaterial(Material* material){ m_Material = material; }
void Object::flagAsChanged(){
	m_Changed = true;
	for(auto child:m_Children)
		child->m_Changed = true;
}
void Object::setName(std::string name){
	std::string oldName = m_Name;
	m_Name = name;
	Resources::Detail::ResourceManagement::m_CurrentScene->getObjects()[name] = this;

	for(auto it = begin(Resources::Detail::ResourceManagement::m_CurrentScene->getObjects()); it != end(Resources::Detail::ResourceManagement::m_CurrentScene->getObjects());){
		if (it->first == oldName)it = Resources::Detail::ResourceManagement::m_CurrentScene->getObjects().erase(it);
	    else++it;
	}
}
glm::vec3 Object::getScreenCoordinates(){
	glm::vec2 windowSize = glm::vec2(static_cast<int>(Resources::getWindow()->getSize().x),static_cast<int>(Resources::getWindow()->getSize().y));
	glm::vec3 objPos = getPosition();
	glm::mat4 V = Resources::getActiveCamera()->getView();
	glm::vec4 viewport = glm::vec4(0.0f,0.0f,static_cast<float>(windowSize.x),static_cast<float>(windowSize.y));
	glm::vec3 screen = glm::project(objPos,V * m_Model,Resources::getActiveCamera()->getProjection(),viewport);

	//check if point is behind
	glm::vec3 viewVector = glm::vec3(V[2][0],V[2][1],V[2][2]);
	float dot = glm::dot(viewVector,objPos);

	float resY = static_cast<float>(windowSize.y-screen.y);


	if(screen.x < 0)
		screen.x = 0;
	else if(screen.x > windowSize.x)
		screen.x = windowSize.x;
	if(resY < 0)
		resY = 0;
	else if(resY > windowSize.y)
		resY = windowSize.y;

	if(dot < 0.0f){
		return glm::vec3(screen.x,resY,screen.z);
	}
	else{
		float fX = windowSize.x - screen.x;
		float fY = windowSize.y - resY;

		if(fX < windowSize.x/2)
			fX = 0;
		else if(fX > windowSize.x/2)
			fX = windowSize.x;
		if(fY < windowSize.y/2)
			fY = 0;
		else if(fY > windowSize.y/2)
			fY = windowSize.y;

		return glm::vec3(fX,fY,screen.z);
	}
}
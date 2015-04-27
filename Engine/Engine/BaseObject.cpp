#include "BaseObject.h"
#include "Engine_Resources.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>

using namespace Engine::Resources;

BaseObject::BaseObject(glm::vec3 pos, std::string name){
	m_Model = glm::mat4(1);
	m_Orientation = glm::quat();
	m_Forward = glm::vec3(0,0,-1);
	m_Right = glm::vec3(1,0,0);
	m_Up = glm::vec3(0,1,0);
	m_Parent = nullptr;
	setPosition(pos);
	setName(name);
}
BaseObject::~BaseObject()
{
}
void BaseObject::update(float dt)
{
}
float BaseObject::getDistance(BaseObject* other){ glm::vec3 vecTo = other->getPosition() - getPosition(); return (abs(glm::length(vecTo))); }
unsigned long long Object::getDistanceLL(BaseObject* other){ glm::vec3 vecTo = other->getPosition() - getPosition(); return static_cast<unsigned long long>(abs(glm::length(vecTo))); }
void BaseObject::_calculateForward(){ m_Forward = glm::normalize(glm::cross(m_Right,m_Up)); }
void BaseObject::_calculateRight(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	m_Right = glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),2 * (x * y + w * z),2 * (x * z - w * y)));
}
void BaseObject::_calculateUp(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	m_Up = glm::normalize(glm::vec3( 2 * (x * y - w * z), 1 - 2 * (x * x + z * z),2 * (y * z + w * x)));
}
void BaseObject::translate(float x, float y, float z,bool local){
	glm::vec3 offset = glm::vec3(0,0,0);
	if(local){
		offset += getForward() * z;
		offset += getRight() * x;
		offset += getUp() * y;
	}
	else{
		offset += glm::vec3(x,y,z);
	}
	this->setPosition(this->getPosition() + offset);
}
void BaseObject::translate(glm::vec3 translation,bool local){ translate(translation.x,translation.y,translation.z,local); }
void BaseObject::rotate(float x, float y, float z){ 
	float threshold = 0.025f;
	if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
		return;
	pitch(x); 
	yaw(y); 
	roll(z); 
}
void BaseObject::rotate(glm::vec3 rotation){ rotate(rotation.x,rotation.y,rotation.z); }
void BaseObject::setPosition(float x, float y, float z){ 
	m_Position.x = x; 
	m_Position.y = y; 
	m_Position.z = z;
}
void BaseObject::setPosition(glm::vec3 position){ setPosition(position.x,position.y,position.z); }
void BaseObject::pitch(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(1,0,0)));
}
void BaseObject::yaw(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(0,1,0)));
}
void BaseObject::roll(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(amount,glm::vec3(0,0,1)));
}
void BaseObject::addChild(BaseObject* child){
	child->m_Parent = this;
	this->m_Children.push_back(child);
}
void BaseObject::setName(std::string name){
	std::string oldName = m_Name;
	m_Name = name;
	Resources::Detail::ResourceManagement::m_CurrentScene->getObjects()[name] = this;
	for(auto it = begin(Resources::Detail::ResourceManagement::m_CurrentScene->getObjects()); it != end(Resources::Detail::ResourceManagement::m_CurrentScene->getObjects());){
		if (it->first == oldName)it = Resources::Detail::ResourceManagement::m_CurrentScene->getObjects().erase(it);
	    else++it;
	}
}
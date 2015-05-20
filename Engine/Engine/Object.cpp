#include "Object.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

Object::Object(glm::vec3 pos, glm::vec3 scl,std::string name,bool isNotLight,Scene* scene){
	m_Forward = glm::vec3(0,0,-1);m_Right = glm::vec3(1,0,0);m_Up = glm::vec3(0,1,0);

	m_Parent = nullptr;

	m_Name = name;
	m_Model = glm::mat4(1);
	m_Orientation = glm::quat();

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
	_updateMatrix();
}
void Object::translate(glm::vec3 translation,bool local){ translate(translation.x,translation.y,translation.z,local); }
void Object::rotate(float x, float y, float z){ 
	float threshold = 0.025f;
	if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
		return;

	if(abs(x) >= threshold)m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-x, glm::vec3(1,0,0)));//pitch
	if(abs(y) >= threshold)m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-y, glm::vec3(0,1,0)));//yaw
	if(abs(z) >= threshold)m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(z,glm::vec3(0,0,1)));  //roll

	_updateMatrix();
	m_Forward = _calculateForward();
	m_Right = _calculateRight();
	m_Up = _calculateUp();
}
void Object::rotate(glm::vec3 rotation){ rotate(rotation.x,rotation.y,rotation.z); }
void Object::scale(float x, float y, float z){
	float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
	m_Scale.x += x * dt; 
	m_Scale.y += y * dt; 
	m_Scale.z += z * dt; 
	_updateMatrix();
}
void Object::scale(glm::vec3 scl){ scale(scl.x,scl.y,scl.z); }
void Object::setPosition(float x, float y, float z){ 
	m_Position.x = x; 
	m_Position.y = y; 
	m_Position.z = z;

	_updateMatrix();
}
void Object::setPosition(glm::vec3 position){ setPosition(position.x,position.y,position.z); }
void Object::setScale(float x, float y, float z){ 
	m_Scale.x = x; 
	m_Scale.y = y; 
	m_Scale.z = z; 
	_updateMatrix();
}
void Object::setScale(glm::vec3 scale){ setScale(scale.x,scale.y,scale.z); }
void Object::update(float dt){

}
void Object::_updateMatrix(){

	glm::mat4 newModel = glm::mat4(1);
	if(m_Parent != nullptr){
		newModel = m_Parent->m_Model;
	}

	newModel = glm::translate(newModel, m_Position);
	newModel *= glm::mat4_cast(m_Orientation);
	newModel = glm::scale(newModel,m_Scale);
	m_Model = newModel;

	for(auto child:m_Children)
		child->_updateMatrix();
}
float Object::getDistance(Object* other){ glm::vec3 vecTo = other->getPosition() - getPosition(); return (abs(glm::length(vecTo))); }
unsigned long long Object::getDistanceLL(Object* other){ glm::vec3 vecTo = other->getPosition() - getPosition(); return static_cast<unsigned long long>(abs(glm::length(vecTo))); }
void Object::addChild(Object* child){
	child->m_Parent = this;
	m_Children.push_back(child);
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
	glm::vec2 windowSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);
	glm::vec3 objPos = getPosition();
	glm::mat4 MV = Resources::getActiveCamera()->getView();
	glm::vec4 viewport = glm::vec4(0,0,windowSize.x,windowSize.y);
	glm::vec3 screen = glm::project(objPos,MV,Resources::getActiveCamera()->getProjection(),viewport);

	//check if point is behind
	float dot = glm::dot(Resources::getActiveCamera()->getViewVector(),glm::vec3(objPos-Resources::getActiveCamera()->getPosition()));

	float resX = static_cast<float>(screen.x);
	float resY = static_cast<float>(windowSize.y-screen.y);

	int inBounds = 1;

	if(screen.x < 0){ resX = 0; inBounds = 0; }
	else if(screen.x > windowSize.x){ resX = windowSize.x; inBounds = 0; }
	if(resY < 0){ resY = 0; inBounds = 0; }
	else if(resY > windowSize.y){ resY = windowSize.y; inBounds = 0; }

	if(dot < 0.0f){
		return glm::vec3(resX,resY,inBounds);
	}
	else{
		inBounds = 0;
		float fX = windowSize.x - screen.x;
		float fY = windowSize.y - resY;

		if(fX < windowSize.x/2){ fX = 0; }
		else if(fX > windowSize.x/2){ fX = windowSize.x; }
		if(fY < windowSize.y/2){ fY = 0; }
		else if(fY > windowSize.y/2){ fY = windowSize.y; }

		return glm::vec3(fX,fY,inBounds);
	}
}
void Object::render(Mesh* m, Material* mat, bool debug){}
void Object::render(bool debug){}
bool Object::rayIntersectSphere(Camera* cam)
{
	return false;
}
#include "Engine_Resources.h"
#include "Math.h"
#include "Camera.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace Engine;
using namespace std;


Entity::Entity(){
	m_ParentID, m_ID = -1;
	m_Components = new uint[ComponentType::_TOTAL];
	for(uint i = 0; i < ComponentType::_TOTAL; ++i){
		m_Components[i] = -1; //apparently assigning -1 to an unsigned int gives it it's max value and can be used to denote an empty component
	}
}
Entity::~Entity(){
	m_ParentID, m_ID = -1;
	for(uint i = 0; i < ComponentType::_TOTAL; ++i){
		epriv::Core::m_Engine->m_ComponentManager->_removeComponent(m_Components[i]);
	}
	delete[] m_Components;

}
Entity* Entity::parent(){
	return epriv::Core::m_Engine->m_ResourceManager->_getEntity(m_ParentID);
}
void Entity::addChild(Entity* child){
	child->m_ParentID = this->m_ID;
}
void Entity::addComponent(ComponentTransform* component){
	if(m_Components[ComponentType::Transform] != -1) return;
	Handle handle = epriv::Core::m_Engine->m_ComponentManager->_addComponent(component,ComponentType::Transform);
	m_Components[ComponentType::Transform] = handle.index;
}
ComponentTransform* Entity::getComponent(ComponentTransform* component){
	return (ComponentTransform*)epriv::Core::m_Engine->m_ComponentManager->_getComponent(m_Components[ComponentType::Transform]);
}
void Entity::addComponent(ComponentModel* component){
	if(m_Components[ComponentType::Model] != -1) return;
	Handle handle = epriv::Core::m_Engine->m_ComponentManager->_addComponent(component,ComponentType::Model);
	m_Components[ComponentType::Model] = handle.index;
}
ComponentModel* Entity::getComponent(ComponentModel* component){
	return (ComponentModel*)epriv::Core::m_Engine->m_ComponentManager->_getComponent(m_Components[ComponentType::Model]);
}











float Object::m_RotationThreshold = 0.0f;
float Object::m_VisibilityThreshold = 1100.0f;
Object::Object(string n,Scene* scene, bool isNotCamera){
    m_Radius = 0;
    m_Parent = nullptr;
    m_IsToBeDestroyed = false;

    setName(n);

    if(isNotCamera){
        if(scene == nullptr){
            scene = Resources::getCurrentScene();
        }
		n = epriv::Core::m_Engine->m_ResourceManager->_buildObjectName(n);
        setName(n);
		epriv::Core::m_Engine->m_ResourceManager->_addObject(this);

        scene->objects().emplace(name(),this);
    }
}
Object::~Object()
{
}
float Object::getDistance(Object* other){ glm::vec3 vecTo = other->getPosition() - getPosition(); return (glm::abs(glm::length(vecTo))); }
unsigned long long Object::getDistanceLL(Object* other){ glm::vec3 vecTo = other->getPosition() - getPosition(); return unsigned long long(glm::abs(glm::length(vecTo))); }
glm::vec3 Object::getScreenCoordinates(){ return Math::getScreenCoordinates(getPosition()); }
void Object::addChild(Object* child){
    child->m_Parent = this;
    m_Children.push_back(child);
}
ObjectBasic::ObjectBasic(glm::vec3 pos,glm::vec3 scl,string name,Scene* scene,bool isNotCameras):Object(name,scene,isNotCameras){
    m_Forward = glm::vec3(0,0,-1);
    m_Right = glm::vec3(1,0,0);
    m_Up = glm::vec3(0,1,0);
    m_Model = glm::mat4(1.0f);
    m_Orientation = glm::quat();
    ObjectBasic::setScale(scl);
    ObjectBasic::setPosition(pos);
}
ObjectBasic::~ObjectBasic(){
}
void ObjectBasic::update(float dt){
    if(m_Parent != nullptr){
        m_Model = m_Parent->getModel();
    }
    else{
        m_Model = glm::mat4(1.0f);
    }
    glm::mat4 translationMatrix = glm::translate(m_Position);
    glm::mat4 rotationMatrix = glm::mat4_cast(m_Orientation);
    glm::mat4 scaleMatrix = glm::scale(m_Scale);

    m_Model = translationMatrix * rotationMatrix * scaleMatrix * m_Model;
}
void ObjectBasic::setPosition(float x,float y,float z){ 
    m_Position.x = x;
    m_Position.y = y;
    m_Position.z = z;

    glm::vec3 parentPos(0.0f);
    if(m_Parent != nullptr){
        parentPos = m_Parent->getPosition();
    }
    m_Model[3][0] = parentPos.x + x;
    m_Model[3][1] = parentPos.y + y;
    m_Model[3][2] = parentPos.z + z;
}
void ObjectBasic::setPosition(glm::vec3 position){ ObjectBasic::setPosition(position.x,position.y,position.z); }
void ObjectBasic::setOrientation(glm::quat q){
    q = glm::normalize(q);
    m_Orientation.x = q.x;
    m_Orientation.y = q.y;
    m_Orientation.z = q.z;
    m_Orientation.w = q.w;

    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::lookAt(glm::vec3 eye,glm::vec3 target,glm::vec3 up){
    Engine::Math::lookAtToQuat(m_Orientation,eye,target,up);
    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::lookAt(Object* o){ ObjectBasic::lookAt(getPosition(), o->getPosition(), o->getUp()); }
void ObjectBasic::alignTo(glm::vec3 direction, float time){ 
    Engine::Math::alignTo(m_Orientation,this,direction,time);
    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::alignTo(Object* other, float time){
    glm::vec3 direction = getPosition() - other->getPosition();
    Engine::Math::alignTo(m_Orientation,this,direction,time);
    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::alignToX(Object* other, float time){ 
    Engine::Math::alignToX(m_Orientation,this,other,time); 
    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::alignToY(Object* other, float time){ 
    Engine::Math::alignToY(m_Orientation,this,other,time); 
    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::alignToZ(Object* other, float time){ 
    Engine::Math::alignToZ(m_Orientation,this,other,time); 
    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
void ObjectBasic::rotate(float x, float y, float z, bool overTime){
    if(overTime){
        x *= Resources::dt(); y *= Resources::dt(); z *= Resources::dt();
    }
    if(abs(x) < Object::m_RotationThreshold && abs(y) < Object::m_RotationThreshold && abs(z) < Object::m_RotationThreshold)
        return;

    if(abs(x) >= Object::m_RotationThreshold) m_Orientation = m_Orientation * (glm::angleAxis(-x, glm::vec3(1,0,0)));   //pitch
    if(abs(y) >= Object::m_RotationThreshold) m_Orientation = m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));   //yaw
    if(abs(z) >= Object::m_RotationThreshold) m_Orientation = m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));   //roll

    Engine::Math::recalculateForwardRightUp(m_Orientation,m_Forward,m_Right,m_Up);
}
glm::vec3 ObjectBasic::getForward(){ return m_Forward; }
glm::vec3 ObjectBasic::getRight(){ return m_Right; }
glm::vec3 ObjectBasic::getUp(){ return m_Up; }
void ObjectBasic::rotate(glm::vec3 rotation,bool overTime){ ObjectBasic::rotate(rotation.x,rotation.y,rotation.z,overTime); }
void ObjectBasic::translate(float x,float y,float z,bool local){   
    glm::vec3 offset = glm::vec3(x,y,z);
    if(local){
        offset = m_Orientation * offset;
    }
    setPosition(getPosition() + offset);
}
void ObjectBasic::translate(glm::vec3 translation,bool local){ ObjectBasic::translate(translation.x,translation.y,translation.z,local); }
void ObjectBasic::scale(float x, float y, float z){
    float dt = Resources::dt();
    m_Scale.x += x * dt; m_Scale.y += y * dt; m_Scale.z += z * dt;
}
void ObjectBasic::scale(glm::vec3 scl){ ObjectBasic::scale(scl.x,scl.y,scl.z); }
void ObjectBasic::setScale(float x, float y, float z){ m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; }
void ObjectBasic::setScale(glm::vec3 scale){ ObjectBasic::setScale(scale.x,scale.y,scale.z); }
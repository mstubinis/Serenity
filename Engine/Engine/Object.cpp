#include "Engine.h"
#include "Engine_Resources.h"
#include "Math.h"
#include "Camera.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

using namespace Engine;

Object::Object(std::string name,Scene* scene, bool isNotCamera){
    m_Radius = 0;
    m_Parent = nullptr;
    m_IsToBeDestroyed = false;
    m_Name = name;
    unsigned int count = 0;

    if(isNotCamera){
        if(scene == nullptr){
            scene = Resources::getCurrentScene();
        }
		m_Name = incrementName(Resources::Detail::ResourceManagement::m_Objects, m_Name);
        scene->getObjects()[m_Name] = this;
        Resources::Detail::ResourceManagement::m_Objects[m_Name] = boost::shared_ptr<Object>(this);
    }
}
Object::~Object()
{
}
glm::nType Object::getDistance(Object* other){ glm::v3 vecTo = other->getPosition() - getPosition(); return (glm::abs(glm::length(vecTo))); }
unsigned long long Object::getDistanceLL(Object* other){ glm::v3 vecTo = other->getPosition() - getPosition(); return static_cast<unsigned long long>(abs(glm::length(vecTo))); }
glm::vec3 Object::getScreenCoordinates(){
    return Math::getScreenCoordinates(glm::vec3(getPosition()));
}
void Object::addChild(Object* child){
    child->m_Parent = this;
    m_Children.push_back(child);
}
void Object::setName(std::string name){
    if(name == m_Name) return;

    std::string oldName = m_Name; m_Name = name;

    Resources::Detail::ResourceManagement::m_CurrentScene->getObjects()[name] = this;
    if(Resources::Detail::ResourceManagement::m_CurrentScene->getObjects().count(oldName)){
        Resources::Detail::ResourceManagement::m_CurrentScene->getObjects().erase(oldName);
    }

    Resources::Detail::ResourceManagement::m_Objects[name] = boost::shared_ptr<Object>(this);
    if(Resources::Detail::ResourceManagement::m_Objects.count(oldName)){
        Resources::Detail::ResourceManagement::m_Objects[oldName].reset();
        Resources::Detail::ResourceManagement::m_Objects.erase(oldName);
    }
}

ObjectBasic::ObjectBasic(glm::v3 pos,glm::vec3 scl,std::string name,Scene* scene,bool isNotCameras):Object(name,scene,isNotCameras){
    m_Forward = glm::v3(0,0,-1);
    m_Right = glm::v3(1,0,0);
    m_Up = glm::v3(0,1,0);
    m_Model = glm::m4(1);
    m_Orientation = glm::quat();
    ObjectBasic::setScale(scl);
    ObjectBasic::setPosition(pos);
    _prevPosition = pos;
}
ObjectBasic::~ObjectBasic(){
}
void ObjectBasic::update(float dt){
    if(m_Parent != nullptr){
        m_Model = m_Parent->getModel();
    }
    else{
        m_Model = glm::m4(1);
    }
    m_Model = glm::translate(m_Model, m_Position);
    m_Model *= glm::m4(glm::mat4_cast(m_Orientation));
    m_Model = glm::scale(m_Model,glm::v3(m_Scale));
}
void ObjectBasic::setPosition(glm::nType x, glm::nType y, glm::nType z){ 
    m_Position.x = x;
    m_Position.y = y;
    m_Position.z = z;

    glm::v3 parentPos(0);
    if(m_Parent != nullptr){
        glm::v3 parentPos = m_Parent->getPosition();
    }
    m_Model[3][0] = parentPos.x + x;
    m_Model[3][1] = parentPos.y + y;
    m_Model[3][2] = parentPos.z + z;
}
void ObjectBasic::setPosition(glm::v3 position){ ObjectBasic::setPosition(position.x,position.y,position.z); }
void ObjectBasic::alignTo(glm::v3 direction, float time, bool overTime){
    Engine::Math::alignTo(m_Orientation,glm::vec3(direction),time,overTime);
}
void ObjectBasic::rotate(float x, float y, float z, bool overTime){
    if(overTime){
        x *= Resources::dt(); y *= Resources::dt(); z *= Resources::dt();
    }
    float threshold = 0;
    if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
        return;

    if(abs(x) >= threshold) m_Orientation = m_Orientation * (glm::angleAxis(-x, glm::vec3(1,0,0)));   //pitch
    if(abs(y) >= threshold) m_Orientation = m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));   //yaw
    if(abs(z) >= threshold) m_Orientation = m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));   //roll

    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::rotate(glm::vec3 rotation,bool overTime){ ObjectBasic::rotate(rotation.x,rotation.y,rotation.z,overTime); }
void ObjectBasic::translate(glm::nType x, glm::nType y, glm::nType z,bool local){
    glm::v3 offset = glm::v3(0);
    if(local){
        offset += getForward() * z;
        offset += getRight() * x;
        offset += getUp() * y;
    }
    else{
        offset += glm::v3(x,y,z);
    }
    setPosition(getPosition() + offset);
}
void ObjectBasic::translate(glm::v3 translation,bool local){ ObjectBasic::translate(translation.x,translation.y,translation.z,local); }
void ObjectBasic::scale(float x, float y, float z){
    float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
    m_Scale.x += x * dt;
    m_Scale.y += y * dt;
    m_Scale.z += z * dt;
}
void ObjectBasic::scale(glm::vec3 scl){ ObjectBasic::scale(scl.x,scl.y,scl.z); }
void ObjectBasic::setScale(float x, float y, float z){ 
    m_Scale.x = x;
    m_Scale.y = y;
    m_Scale.z = z;
}
void ObjectBasic::setScale(glm::vec3 scale){ ObjectBasic::setScale(scale.x,scale.y,scale.z); }
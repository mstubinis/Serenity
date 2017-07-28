#include "Engine.h"
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

float Object::m_RotationThreshold = 0.0f;
Object::Object(std::string n,Scene* scene, bool isNotCamera){
    m_Radius = 0;
    m_Parent = nullptr;
    m_IsToBeDestroyed = false;
    setName(n);

    if(isNotCamera){
        if(scene == nullptr){
            scene = Resources::getCurrentScene();
        }
        setName(Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_Objects, name()));
        Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Objects,name(),boost::shared_ptr<Object>(this));

        scene->objects().emplace(name(),this);
    }
}
Object::~Object()
{
}
glm::num Object::getDistance(Object* other){ glm::v3 vecTo = other->getPosition() - getPosition(); return (glm::abs(glm::length(vecTo))); }
unsigned long long Object::getDistanceLL(Object* other){ glm::v3 vecTo = other->getPosition() - getPosition(); return unsigned long long(glm::abs(glm::length(vecTo))); }
glm::vec3 Object::getScreenCoordinates(){ return Math::getScreenCoordinates(glm::vec3(getPosition())); }
void Object::addChild(Object* child){
    child->m_Parent = this;
    m_Children.push_back(child);
}
ObjectBasic::ObjectBasic(glm::v3 pos,glm::vec3 scl,std::string name,Scene* scene,bool isNotCameras):Object(name,scene,isNotCameras){
    m_Forward = glm::v3(0,0,-1);
    m_Right = glm::v3(1,0,0);
    m_Up = glm::v3(0,1,0);
    m_Model = glm::m4(1);
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
        m_Model = glm::m4(1.0f);
    }
    glm::m4 translationMatrix = glm::translate(m_Position);
    glm::m4 rotationMatrix = glm::m4(glm::mat4_cast(m_Orientation));
    glm::m4 scaleMatrix = glm::scale(glm::v3(m_Scale));

    m_Model = translationMatrix * rotationMatrix * scaleMatrix * m_Model;
}
void ObjectBasic::setPosition(glm::num x, glm::num y, glm::num z){ 
    m_Position.x = x;
    m_Position.y = y;
    m_Position.z = z;

    glm::v3 parentPos(0);
    if(m_Parent != nullptr){
        parentPos = m_Parent->getPosition();
    }
    m_Model[3][0] = parentPos.x + x;
    m_Model[3][1] = parentPos.y + y;
    m_Model[3][2] = parentPos.z + z;
}
void ObjectBasic::setPosition(glm::v3 position){ ObjectBasic::setPosition(position.x,position.y,position.z); }
void ObjectBasic::setOrientation(glm::quat q){ 
    m_Orientation = q;
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::lookAt(glm::v3 eye,glm::v3 target,glm::v3 up){
    Engine::Math::lookAtToQuat(m_Orientation,eye,target,up);
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::lookAt(Object* o){ ObjectBasic::lookAt(getPosition(), o->getPosition(), o->getUp()); }
void ObjectBasic::alignTo(glm::v3 direction, float time){ 
    Engine::Math::alignTo(m_Orientation,this,glm::vec3(direction),time);
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::alignTo(Object* other, float time){
    glm::vec3 direction = glm::vec3(getPosition() - other->getPosition());
    Engine::Math::alignTo(m_Orientation,this,direction,time);
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::alignToX(Object* other, float time){ 
    Engine::Math::alignToX(m_Orientation,this,other,time); 
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::alignToY(Object* other, float time){ 
    Engine::Math::alignToY(m_Orientation,this,other,time); 
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::alignToZ(Object* other, float time){ 
    Engine::Math::alignToZ(m_Orientation,this,other,time); 
    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
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

    m_Forward = Engine::Math::getForward(m_Orientation);
    m_Right = Engine::Math::getRight(m_Orientation);
    m_Up = Engine::Math::getUp(m_Orientation);
}
void ObjectBasic::rotate(glm::vec3 rotation,bool overTime){ ObjectBasic::rotate(rotation.x,rotation.y,rotation.z,overTime); }
void ObjectBasic::translate(glm::num x, glm::num y, glm::num z,bool local){   
    glm::v3 offset = glm::v3(0.0f);
    if(local){
        offset += m_Forward * z;
        offset += m_Right * x;
        offset += m_Up * y;
    }
    else{
        offset += glm::v3(x,y,z);
    }
    setPosition(getPosition() + offset);
}
void ObjectBasic::translate(glm::v3 translation,bool local){ ObjectBasic::translate(translation.x,translation.y,translation.z,local); }
void ObjectBasic::scale(float x, float y, float z){
    float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
    m_Scale.x += x * dt; m_Scale.y += y * dt; m_Scale.z += z * dt;
}
void ObjectBasic::scale(glm::vec3 scl){ ObjectBasic::scale(scl.x,scl.y,scl.z); }
void ObjectBasic::setScale(float x, float y, float z){ m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; }
void ObjectBasic::setScale(glm::vec3 scale){ ObjectBasic::setScale(scale.x,scale.y,scale.z); }

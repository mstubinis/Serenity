#include "ecs/Entity.h"
#include "core/engine/Engine.h"
#include "core/Scene.h"

using namespace Engine;
using namespace std;

Entity Entity::_null = Entity(0);


Entity::Entity(){
    ID = 0; sceneID = 0;
}
Entity::Entity(uint _id){
    ID = _id;
    sceneID = 0;
}
Entity::Entity(uint _id, Scene& _scene){
    ID = _id;
    sceneID = _scene.id();
}

Entity::Entity(const Entity& other) {
    ID = other.ID;
    sceneID = other.sceneID;
}
Entity& Entity::operator=(const Entity& other) {
    if (&other == this)
        return *this;
    ID = other.ID;
    sceneID = other.sceneID;
    return *this;
}

Entity::~Entity() { 
    ID = 0; sceneID = 0; 
}
inline Entity::operator uint() const { 
    return sceneID << 24 | ID; 
}
Scene& Entity::scene() {
    return epriv::Core::m_Engine->m_ResourceManager._getSceneByID(sceneID);
}
bool Entity::null() { 
    if (ID == 0) return true; 
    return false; 
}
void Entity::move(Scene& _scene) {
    if (_scene.id() == sceneID) return;
    //do whatever is needed
    //for(auto system: allComponentSystems){
    //	system.removeComponent(this);
    //}
    sceneID = _scene.id();
}


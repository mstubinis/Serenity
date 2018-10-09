#include "ecs/Entity.h"
#include "ecs/EntitySerialization.h"
#include "core/engine/Engine.h"
#include "core/Scene.h"

#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)

using namespace Engine;
using namespace std;

Entity Entity::_null = Entity(0,0,0);

Scene& Entity::scene() {
    epriv::EntitySerialization _s(*this);
    return epriv::Core::m_Engine->m_ResourceManager._getSceneByID(_s.sceneID);
}
bool Entity::null() { return data == 0 ? true : false; }
void Entity::move(Scene& _scene) {
    epriv::EntitySerialization _s(*this);
    if (_scene.id() == _s.sceneID) return;
    //do whatever is needed
    serialize(_s.ID, _scene.id(), _s.versionID);
}



ECS& epriv::InternalEntityPublicInterface::GetECS(Entity& _entity) {
    EntitySerialization _s(_entity);
    Scene& s = epriv::Core::m_Engine->m_ResourceManager._getSceneByID(_s.sceneID);
    return Engine::epriv::InternalScenePublicInterface::GetECS(s);
}
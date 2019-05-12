#include <ecs/Entity.h>

using namespace Engine;
using namespace std;

Entity Entity::_null = Entity(0,0,0);

Scene& Entity::scene() {
    epriv::EntitySerialization _s(*this);
    return epriv::Core::m_Engine->m_ResourceManager._getSceneByID(_s.sceneID);
}
void Entity::move(Scene& _scene) {
    epriv::EntitySerialization _s(*this);
    if (_scene.id() == _s.sceneID) return;
    //do whatever is needed
    serialize(_s.ID, _scene.id(), _s.versionID);
}
void Entity::destroy() {
    Scene& s = scene();
    auto& _ecs = Engine::epriv::InternalScenePublicInterface::GetECS(s);
    _ecs.removeEntity(*this);
}
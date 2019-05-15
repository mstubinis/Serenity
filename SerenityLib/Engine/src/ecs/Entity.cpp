#include <ecs/Entity.h>

using namespace Engine;
using namespace std;

Entity Entity::_null = Entity(0,0,0);

Scene& Entity::scene() {
    EntityDataRequest dataRequest(*this);
    return epriv::Core::m_Engine->m_ResourceManager._getSceneByID(dataRequest.sceneID);
}
void Entity::move(Scene& _scene) {
    EntityDataRequest dataRequest(*this);
    if (_scene.id() == dataRequest.sceneID)
        return;
    process(dataRequest.ID, _scene.id(), dataRequest.versionID);
}
void Entity::destroy() {
    Scene& s = scene();
    auto& _ecs = epriv::InternalScenePublicInterface::GetECS(s);
    _ecs.removeEntity(*this);
}
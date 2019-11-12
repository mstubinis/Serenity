#include <ecs/Entity.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/Engine.h>

using namespace Engine;
using namespace std;

Entity Entity::_null = Entity(0,0,0);

Entity::Entity() {
    data = 0;
}
Entity::Entity(const uint entityID, const uint sceneID, const uint versionID) {
    process(entityID, sceneID, versionID);
}
Entity::~Entity() {
    data = 0;
}

Scene& Entity::scene() {
	const EntityDataRequest dataRequest(*this);
    return epriv::Core::m_Engine->m_ResourceManager._getSceneByID(dataRequest.sceneID);
}
void Entity::move(const Scene& scene) {
	const EntityDataRequest dataRequest(*this);
    const auto sceneID = scene.id();
    if (sceneID == dataRequest.sceneID)
        return;
    process(dataRequest.ID, sceneID, dataRequest.versionID);
}
void Entity::destroy() {
    Scene& _scene = scene();
    epriv::InternalScenePublicInterface::CleanECS(_scene, data);
    auto& ecs = epriv::InternalScenePublicInterface::GetECS(_scene);
    ecs.removeEntity(*this);
}
void Entity::process(const uint entityID, const uint sceneID, const uint versionID) {
    data = versionID << 28 | sceneID << 21 | entityID;
}
const bool Entity::operator==(const Entity& other) const {
    return (data == other.data) ? true : false;
}
const bool Entity::operator!=(const Entity& other) const {
    return (data == other.data) ? false : true;
}
const bool Entity::null() {
    return (data == 0) ? true : false;
}

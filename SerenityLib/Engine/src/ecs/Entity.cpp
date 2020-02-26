#include <ecs/Entity.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>
#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

Entity Entity::null_ = Entity(0,0,0);

Entity::Entity() {
}
Entity::Entity(const unsigned int entityID, const unsigned int sceneID, const unsigned int versionID) {
    process(entityID, sceneID, versionID);
}
Entity::~Entity() {
}
Entity::Entity(const Entity& other) {
    data = other.data;
}
Entity& Entity::operator=(const Entity& other) {
    if (&other != this) {
        data = other.data;
    }
    return *this;
}
Entity::Entity(Entity&& other) noexcept {
    data = std::move(other.data);
}
Entity& Entity::operator=(Entity&& other) noexcept {
    if (&other != this) {
        data = std::move(other.data);
    }
    return *this;
}
const std::uint32_t Entity::id() const {
    //const std::uint32_t id = (data & 4'194'303) >> 0;
    //return id;  // = 21 (2 ^ 22 = 419,304)

    const EntityDataRequest dataRequest(*this);
    return dataRequest.ID;
}
const std::uint32_t Entity::sceneID() const {
    //const std::uint32_t scene_id = (data & 534'773'760) >> 21;
    //return scene_id; //+7 = 28    (2 ^ 29 = 536,870,912)

    const EntityDataRequest dataRequest(*this);
    return dataRequest.sceneID;
}
void Entity::addChild(const Entity& child) const {
    auto* body = getComponent<ComponentBody>();
    if (body)
        body->addChild(child);
}
void Entity::removeChild(const Entity& child) const {
    auto* body = getComponent<ComponentBody>();
    if(body)
        body->removeChild(child);
}
const bool Entity::hasParent() const {
    auto* body = getComponent<ComponentBody>();
    if (body)
        return body->hasParent();
    return false;
}


Scene& Entity::scene() const {
	const EntityDataRequest dataRequest(*this);
    return priv::Core::m_Engine->m_ResourceManager._getSceneByID(dataRequest.sceneID);
}
void Entity::move(const Scene& scene) {
	const EntityDataRequest dataRequest(*this);
    const auto sceneID = scene.id();
    if (sceneID == dataRequest.sceneID) {
        return;
    }
    process(dataRequest.ID, sceneID, dataRequest.versionID);
}
void Entity::destroy() {
    Scene& _scene = scene();
    priv::InternalScenePublicInterface::CleanECS(_scene, data);
    auto& ecs = priv::InternalScenePublicInterface::GetECS(_scene);
    ecs.removeEntity(*this);
}
void Entity::process(const unsigned int entityID, const unsigned int sceneID, const unsigned int versionID) {
    data = versionID << 28 | sceneID << 21 | entityID;
}
const bool Entity::operator==(const Entity& other) const {
    return (data == other.data) ? true : false;
}
const bool Entity::operator!=(const Entity& other) const {
    return (data == other.data) ? false : true;
}
const bool Entity::null() const {
    return (data == 0) ? true : false;
}

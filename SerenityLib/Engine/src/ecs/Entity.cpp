#include <ecs/Entity.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>
#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

Entity Entity::null_ = Entity(0,0,0);

Entity::Entity(Scene& scene) {
    data = scene.createEntity().data;
}
Entity::Entity(const unsigned int entityID, const unsigned int sceneID, const unsigned int versionID) {
    data = versionID << 28 | sceneID << 21 | entityID;
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
    data = std::move(other.data);
    return *this;
}


const std::uint32_t Entity::id() const {
    const EntityDataRequest dataRequest(*this);
    return dataRequest.ID;
}
const std::uint32_t Entity::sceneID() const {
    const EntityDataRequest dataRequest(*this);
    return dataRequest.sceneID;
}
const std::uint32_t Entity::versionID() const {
    const EntityDataRequest dataRequest(*this);
    return dataRequest.versionID;
}

void Entity::addChild(const Entity child) const {
    const auto* body = getComponent<ComponentBody>();
    if (body) {
        body->addChild(child);
    }
}
void Entity::removeChild(const Entity child) const {
    const auto* body = getComponent<ComponentBody>();
    if (body) {
        body->removeChild(child);
    }
}
const bool Entity::hasParent() const {
    const auto* body = getComponent<ComponentBody>();
    if (body) {
        return body->hasParent();
    }
    return false;
}


Scene& Entity::scene() const {
	const EntityDataRequest dataRequest(*this);
    return priv::Core::m_Engine->m_ResourceManager._getSceneByID(dataRequest.sceneID);
}
void Entity::destroy() {
    Scene& scene_ = scene();
    priv::InternalScenePublicInterface::CleanECS(scene_, data);
    priv::InternalScenePublicInterface::GetECS(scene_).removeEntity(*this);
}
const bool Entity::operator==(const Entity other) const {
    return (data == other.data);
}
const bool Entity::operator!=(const Entity other) const {
    return !(data == other.data);
}
const bool Entity::null() const {
    return (data == 0);
}

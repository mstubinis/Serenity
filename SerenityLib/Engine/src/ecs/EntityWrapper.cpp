#include <ecs/EntityWrapper.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace std;
using namespace Engine;


EntityWrapper::EntityWrapper(Scene& scene) {
    Scene* scene_ptr = &scene;
    if (!scene_ptr) {
        scene_ptr = Resources::getCurrentScene();
    }
    m_Entity = scene_ptr->createEntity();
}
EntityWrapper::~EntityWrapper() {

}
EntityWrapper::EntityWrapper(EntityWrapper&& other) noexcept {
    m_Entity = std::move(other.m_Entity);
}
EntityWrapper& EntityWrapper::operator=(EntityWrapper&& other) noexcept {
    m_Entity = std::move(other.m_Entity);
    return *this;
}
void EntityWrapper::destroy() {
    m_Entity.destroy();
}
const Entity& EntityWrapper::entity() const {
    return m_Entity;
}
const bool EntityWrapper::null() const {
    return m_Entity.null();
}
const std::uint32_t EntityWrapper::id() const {
    return m_Entity.id();
}
const std::uint32_t EntityWrapper::sceneID() const {
    return m_Entity.sceneID();
}
void EntityWrapper::addChild(const Entity& child) const {
    m_Entity.addChild(child);
}
void EntityWrapper::removeChild(const Entity& child) const {
    m_Entity.removeChild(child);
}

void EntityWrapper::addChild(const EntityWrapper& child) const {
    m_Entity.addChild(child.entity());
}
void EntityWrapper::removeChild(const EntityWrapper& child) const {
    m_Entity.removeChild(child.entity());
}
const bool EntityWrapper::hasParent() const {
    return m_Entity.hasParent();
}
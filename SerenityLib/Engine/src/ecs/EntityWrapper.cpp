#include <ecs/EntityWrapper.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace std;
using namespace Engine;

EntityWrapper::EntityWrapper(Scene& scene) {
    Scene* scene_ = &scene;
    if (!scene_)
        scene_ = Resources::getCurrentScene();
    m_Entity = scene_->createEntity();
}
EntityWrapper::~EntityWrapper() {
    m_Entity = Entity::null_;
}
EntityWrapper::EntityWrapper(const EntityWrapper& other) {
    m_Entity = other.m_Entity;
}
EntityWrapper& EntityWrapper::operator=(const EntityWrapper& other) {
    if (&other == this)
        return *this;
    m_Entity = other.m_Entity;
    return *this;
}
EntityWrapper::EntityWrapper(EntityWrapper&& other) noexcept {
    using std::swap;
    swap(m_Entity, other.m_Entity);
}
EntityWrapper& EntityWrapper::operator=(EntityWrapper&& other) noexcept {
    using std::swap;
    swap(m_Entity, other.m_Entity);
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
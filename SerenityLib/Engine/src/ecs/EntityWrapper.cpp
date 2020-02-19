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
}
EntityWrapper::EntityWrapper(EntityWrapper&& other) noexcept {
    m_Entity = std::move(other.m_Entity);
}
EntityWrapper& EntityWrapper::operator=(EntityWrapper&& other) noexcept {
    if (&other != this) {
        m_Entity = std::move(other.m_Entity);
    }
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
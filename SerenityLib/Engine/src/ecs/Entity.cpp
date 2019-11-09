#include <ecs/Entity.h>

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



EntityWrapper::EntityWrapper(Scene& scene) {
    Scene* scene_ = &scene;
    if (!scene_)
        scene_ = Resources::getCurrentScene();
    m_Entity = scene_->createEntity();
}
EntityWrapper::~EntityWrapper() {
    m_Entity = Entity::_null;
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
Entity& EntityWrapper::entity() {
    return m_Entity;
}
const bool EntityWrapper::null() {
    return m_Entity.null();
}
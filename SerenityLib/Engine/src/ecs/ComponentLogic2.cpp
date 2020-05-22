#include <ecs/ComponentLogic2.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic2::ComponentLogic2(const Entity entity) {
    m_Owner = entity;
}
ComponentLogic2::~ComponentLogic2() {
}
ComponentLogic2::ComponentLogic2(ComponentLogic2&& other) noexcept {
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner        = std::move(other.m_Owner);

    m_Functor.swap(other.m_Functor);
}
ComponentLogic2& ComponentLogic2::operator=(ComponentLogic2&& other) noexcept {
    if (&other != this) {
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner        = std::move(other.m_Owner);

        m_Functor.swap(other.m_Functor);
    }
    return *this;
}
const Entity ComponentLogic2::getOwner() const {
    return m_Owner;
}
void ComponentLogic2::setFunctor(std::function<void(const ComponentLogic2*, const float)> functor) {
    m_Functor = functor;
}
void ComponentLogic2::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic2::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic2::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic2::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentLogic2::call(const float dt) const { 
    //if (m_Functor) {
        m_Functor(this, dt);
    //}
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic2_UpdateFunction final {void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentLogic2>*>(componentPool);
    const auto& components = pool.data();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct priv::ComponentLogic2_ComponentAddedToEntityFunction final {void operator()(void* system, void* component, Entity& entity) const {
}};
struct priv::ComponentLogic2_ComponentRemovedFromEntityFunction final { void operator()(void* system, Entity& entity) const {
}};
struct priv::ComponentLogic2_EntityAddedToSceneFunction final {void operator()(void* system, void* componentPool, Entity& entity, Scene& scene) const {
}};
struct priv::ComponentLogic2_SceneEnteredFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {

}};
struct priv::ComponentLogic2_SceneLeftFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {

}};

ComponentLogic2_System_CI::ComponentLogic2_System_CI() {
    setUpdateFunction(ComponentLogic2_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic2_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentLogic2_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic2_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic2_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic2_SceneLeftFunction());
}

#pragma endregion
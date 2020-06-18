#include <ecs/ComponentLogic3.h>
#include <ecs/ECSComponentPool.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic3::ComponentLogic3(Entity entity) {
    m_Owner = entity;
}
ComponentLogic3::~ComponentLogic3() {

}
ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept {
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner        = std::move(other.m_Owner);
    m_Functor      = std::move(other.m_Functor);
}
ComponentLogic3& ComponentLogic3::operator=(ComponentLogic3&& other) noexcept {
    if (&other != this) {
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner        = std::move(other.m_Owner);
        m_Functor      = std::move(other.m_Functor);
    }
    return *this;
}
const Entity ComponentLogic3::getOwner() const {
    return m_Owner;
}
void ComponentLogic3::setFunctor(std::function<void(const ComponentLogic3*, const float)> functor) {
    m_Functor.setFunctor(functor);
}
void ComponentLogic3::setFunctor(luabridge::LuaRef luaFunction) {
    m_Functor.setFunctor(luaFunction);
}
void ComponentLogic3::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic3::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic3::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic3::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentLogic3::call(const float dt) const { 
    m_Functor(this, dt);
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic3_UpdateFunction final {void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentLogic3>*>(componentPool);
    const auto& components = pool.data();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct priv::ComponentLogic3_ComponentAddedToEntityFunction final {void operator()(void* system, void* component, Entity entity) const {
}};
struct priv::ComponentLogic3_ComponentRemovedFromEntityFunction final { void operator()(void* system, Entity entity) const {
}};
struct priv::ComponentLogic3_EntityAddedToSceneFunction final {void operator()(void* system, void* componentPool, Entity entity, Scene& scene) const {
}};
struct priv::ComponentLogic3_SceneEnteredFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {

}};
struct priv::ComponentLogic3_SceneLeftFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {

}};

ComponentLogic3_System_CI::ComponentLogic3_System_CI() {
    setUpdateFunction(ComponentLogic3_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic3_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentLogic3_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic3_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic3_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic3_SceneLeftFunction());
}

#pragma endregion
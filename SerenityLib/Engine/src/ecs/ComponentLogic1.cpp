#include <ecs/ComponentLogic1.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic1::ComponentLogic1(const Entity entity) {
    m_Owner = entity;
}
ComponentLogic1::~ComponentLogic1() {

}
ComponentLogic1::ComponentLogic1(ComponentLogic1&& other) noexcept {
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner        = std::move(other.m_Owner);
    m_Functor      = std::move(other.m_Functor);
}
ComponentLogic1& ComponentLogic1::operator=(ComponentLogic1&& other) noexcept {
    if (&other != this) {
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner        = std::move(other.m_Owner);
        m_Functor      = std::move(other.m_Functor);
    }
    return *this;
}
const Entity ComponentLogic1::getOwner() const {
    return m_Owner;
}
void ComponentLogic1::setFunctor(std::function<void(const ComponentLogic1*, const float)> functor) {
    m_Functor.setFunctor(functor);
}
void ComponentLogic1::setFunctor(luabridge::LuaRef luaFunction) {
    if (!luaFunction.isNil() && luaFunction.isFunction()) {
        m_Functor.setFunctor(luaFunction);
    }
}
void ComponentLogic1::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic1::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic1::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic1::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentLogic1::call(const float dt) const { 
    m_Functor.call(this, dt);
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic1_UpdateFunction final {void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentLogic1>*>(componentPool);
    const auto& components = pool.data();
    for (auto& component: components) {
        component.call(dt);
    }
}};
struct priv::ComponentLogic1_ComponentAddedToEntityFunction final {void operator()(void* system, void* component, Entity& entity) const {
}};
struct priv::ComponentLogic1_ComponentRemovedFromEntityFunction final { void operator()(void* system, Entity& entity) const {
}};
struct priv::ComponentLogic1_EntityAddedToSceneFunction final {void operator()(void* system, void* componentPool, Entity& entity, Scene& scene) const {
}};
struct priv::ComponentLogic1_SceneEnteredFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {

}};
struct priv::ComponentLogic1_SceneLeftFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {

}};

ComponentLogic1_System_CI::ComponentLogic1_System_CI() {
    setUpdateFunction(ComponentLogic1_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic1_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentLogic1_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic1_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic1_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic1_SceneLeftFunction());
}

#pragma endregion
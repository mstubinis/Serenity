#include <core/engine/utils/PrecompiledHeader.h>
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

ComponentLogic3_System_CI::ComponentLogic3_System_CI() {
    setUpdateFunction([](void* system, void* componentPool, const float dt, Scene& scene) {
        auto& pool = *static_cast<ECSComponentPool<Entity, ComponentLogic3>*>(componentPool);
        const auto& components = pool.data();
        for (auto& component : components) {
            component.call(dt);
        }
    });
    setOnComponentAddedToEntityFunction([](void* system, void* component, Entity entity) {
    });
    setOnComponentRemovedFromEntityFunction([](void* system, Entity entity) {
    });
    setOnEntityAddedToSceneFunction([](void* system, void* componentPool, Entity entity, Scene& scene) {
    });
    setOnSceneEnteredFunction([](void* system, void* componentPool, Scene& scene) {
    });
    setOnSceneLeftFunction([](void* system, void* componentPool, Scene& scene) {
    });
}

#pragma endregion
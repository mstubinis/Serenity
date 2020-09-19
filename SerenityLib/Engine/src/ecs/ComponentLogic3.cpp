#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/ComponentLogic3.h>
#include <ecs/ECSComponentPool.h>

#pragma region Component

ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept
    : m_UserPointer1(std::exchange(other.m_UserPointer1, nullptr))
    , m_UserPointer2(std::exchange(other.m_UserPointer2, nullptr))
    , m_Owner(std::move(other.m_Owner))
    , m_Functor(std::move(other.m_Functor))
{
    m_UserPointer = std::exchange(other.m_UserPointer, nullptr);
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
void ComponentLogic3::setFunctor(luabridge::LuaRef luaFunction) noexcept {
    m_Functor.setFunctor(luaFunction);
}
void ComponentLogic3::call(const float dt) const noexcept {
    m_Functor(this, dt);
}

#pragma endregion

#pragma region System

ComponentLogic3_System_CI::ComponentLogic3_System_CI() {
    setUpdateFunction([](void* system, void* componentPool, const float dt, Scene& scene) {
        auto& pool = *static_cast<Engine::priv::ECSComponentPool<Entity, ComponentLogic3>*>(componentPool);
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
#include <serenity/ecs/components/ComponentLogic3.h>
#include <serenity/ecs/ECSComponentPool.h>

#pragma region Component

ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept
    : m_Functor{ std::move(other.m_Functor) }
    , m_UserPointer { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1{ std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2{ std::exchange(other.m_UserPointer2, nullptr) }
    , m_Owner{ std::exchange(other.m_Owner, Entity{}) }
{
}
ComponentLogic3& ComponentLogic3::operator=(ComponentLogic3&& other) noexcept {
    if (this != &other) {
        m_Functor      = std::move(other.m_Functor);
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner        = std::exchange(other.m_Owner, Entity{});
    }
    return *this;
}
void ComponentLogic3::call(const float dt) const noexcept {
    m_Functor(this, dt);
}

#pragma endregion

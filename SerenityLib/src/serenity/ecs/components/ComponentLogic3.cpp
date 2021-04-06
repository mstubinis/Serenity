#include <serenity/ecs/components/ComponentLogic3.h>
#include <serenity/ecs/ECSComponentPool.h>

#pragma region Component

ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept
    : m_UserPointer { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1{ std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2{ std::exchange(other.m_UserPointer2, nullptr) }
    , m_Owner       { std::move(other.m_Owner) }
    , m_Functor     { std::move(other.m_Functor) }
{
}
ComponentLogic3& ComponentLogic3::operator=(ComponentLogic3&& other) noexcept {
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner        = std::move(other.m_Owner);
    m_Functor      = std::move(other.m_Functor);
    return *this;
}
void ComponentLogic3::call(const float dt) const noexcept {
    m_Functor(this, dt);
}

#pragma endregion

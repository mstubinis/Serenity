#include <serenity/ecs/components/ComponentLogic.h>
#include <serenity/ecs/ECSComponentPool.h>

#pragma region Component

ComponentLogic::ComponentLogic(ComponentLogic&& other) noexcept 
    : m_UserPointer { std::exchange(other.m_UserPointer, nullptr) }
    , m_UserPointer1{ std::exchange(other.m_UserPointer1, nullptr) }
    , m_UserPointer2{ std::exchange(other.m_UserPointer2, nullptr) }
    , m_Owner       { std::exchange(other.m_Owner, Entity{}) }
    , m_Functor     { std::move(other.m_Functor) }
{
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner            = std::exchange(other.m_Owner, Entity{});
    m_Functor          = std::move(other.m_Functor);
    return *this;
}

void ComponentLogic::call(const float dt) const noexcept {
    m_Functor(this, dt);
}

#pragma endregion

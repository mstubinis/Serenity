#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

class ComponentLogic;
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic : public Engine::UserPointer {
    private:
        Entity                                             m_Owner;
        void*                                              m_UserPointer1  = nullptr;
        void*                                              m_UserPointer2  = nullptr;
        LuaCallableFunction<ComponentLogic, const float>   m_Functor;
    public:
        ComponentLogic(Entity entity);
        template<typename T> 
        ComponentLogic(Entity entity, T&& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
            m_Owner        = entity;
            m_UserPointer  = Ptr1;
            m_UserPointer1 = Ptr2;
            m_UserPointer2 = Ptr3;
            setFunctor(std::move(Functor));
        }
        ComponentLogic(const ComponentLogic& other) = delete;
        ComponentLogic& operator=(const ComponentLogic& other) = delete;
        ComponentLogic(ComponentLogic&& other) noexcept;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept;

        ~ComponentLogic();

        inline CONSTEXPR Entity getOwner() const noexcept { return m_Owner; }
        void call(const float dt) const;

        void setFunctor(std::function<void(const ComponentLogic*, const float)>&& functor);
        void setFunctor(luabridge::LuaRef luaFunction);

        inline void setUserPointer1(void* UserPointer1) noexcept { m_UserPointer1 = UserPointer1; }
        inline void setUserPointer2(void* UserPointer2) noexcept { m_UserPointer2 = UserPointer2; }
        
        inline CONSTEXPR void* getUserPointer1() const noexcept { return m_UserPointer1; }
        inline CONSTEXPR void* getUserPointer2() const noexcept { return m_UserPointer2; }
};

class ComponentLogic_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic_System_CI();
        ~ComponentLogic_System_CI() = default;
};

#endif
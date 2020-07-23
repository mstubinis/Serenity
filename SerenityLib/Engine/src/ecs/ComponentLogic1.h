#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_1_H
#define ENGINE_ECS_COMPONENT_LOGIC_1_H

class ComponentLogic1;
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic1 : public Engine::UserPointer {
    private:
        Entity                                             m_Owner;
        void*                                              m_UserPointer1 = nullptr;
        void*                                              m_UserPointer2 = nullptr;
        LuaCallableFunction<ComponentLogic1, const float>  m_Functor;
    public:
        ComponentLogic1(Entity entity);
        template<typename T> 
        ComponentLogic1(Entity entity, T&& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
            m_Owner        = entity;
            m_UserPointer  = Ptr1;
            m_UserPointer1 = Ptr2;
            m_UserPointer2 = Ptr3;
            setFunctor(std::move(Functor));
        }
        ComponentLogic1(const ComponentLogic1& other) = delete;
        ComponentLogic1& operator=(const ComponentLogic1& other) = delete;
        ComponentLogic1(ComponentLogic1&& other) noexcept;
        ComponentLogic1& operator=(ComponentLogic1&& other) noexcept;

        ~ComponentLogic1();

        inline CONSTEXPR Entity getOwner() const noexcept { return m_Owner; }
        void call(const float dt) const;

        void setFunctor(std::function<void(const ComponentLogic1*, const float)>&& functor);
        void setFunctor(luabridge::LuaRef luaFunction);

        inline void setUserPointer1(void* UserPointer1) noexcept { m_UserPointer1 = UserPointer1; }
        inline void setUserPointer2(void* UserPointer2) noexcept { m_UserPointer2 = UserPointer2; }

        inline CONSTEXPR void* getUserPointer1() const noexcept { return m_UserPointer1; }
        inline CONSTEXPR void* getUserPointer2() const noexcept { return m_UserPointer2; }
};

class ComponentLogic1_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic1_System_CI();
        ~ComponentLogic1_System_CI() = default;
};
#endif
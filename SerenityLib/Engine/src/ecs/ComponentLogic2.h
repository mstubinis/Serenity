#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

class ComponentLogic2;
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic2 : public Engine::UserPointer, public Engine::NonCopyable {
    using c_function = void(*)(const ComponentLogic2*, const float);
    private:
        Entity                                             m_Owner;
        void*                                              m_UserPointer1 = nullptr;
        void*                                              m_UserPointer2 = nullptr;
        LuaCallableFunction<ComponentLogic2, const float>  m_Functor;
    public:
        ComponentLogic2(Entity entity) 
            : m_Owner(entity)
        {}
        ComponentLogic2(Entity entity, c_function&& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) 
            : m_Owner(entity)
            , m_UserPointer1(Ptr2)
            , m_UserPointer2(Ptr3)
        {
            m_UserPointer  = Ptr1;
            setFunctor(std::move(Functor));
        }
        ComponentLogic2(Entity entity, luabridge::LuaRef luaFunction, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_Owner(entity)
            , m_UserPointer1(Ptr2)
            , m_UserPointer2(Ptr3)
        {
            m_UserPointer = Ptr1;
            setFunctor(luaFunction);
        }
        ComponentLogic2(ComponentLogic2&& other) noexcept;
        ComponentLogic2& operator=(ComponentLogic2&& other) noexcept;

        ~ComponentLogic2() = default;

        inline CONSTEXPR Entity getOwner() const noexcept { return m_Owner; }
        void call(const float dt) const noexcept;
        
        inline void setFunctor(c_function&& functor) noexcept { m_Functor.setFunctor(std::move(functor)); }
        inline void setFunctor(luabridge::LuaRef luaFunction) noexcept {
            ASSERT(!luaFunction.isNil() && luaFunction.isFunction(), "ComponentLogic2::setFunctor(luabridge::LuaRef): lua ref is invalid!");
            m_Functor.setFunctor(luaFunction);
        }

        inline void setUserPointer1(void* UserPointer1) noexcept { m_UserPointer1 = UserPointer1; }
        inline void setUserPointer2(void* UserPointer2) noexcept { m_UserPointer2 = UserPointer2; }

        inline CONSTEXPR void* getUserPointer1() const noexcept { return m_UserPointer1; }
        inline CONSTEXPR void* getUserPointer2() const noexcept { return m_UserPointer2; }
};

class ComponentLogic2_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic2_System_CI();
        ~ComponentLogic2_System_CI() = default;
};

#endif
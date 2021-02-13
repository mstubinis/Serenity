#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

class ComponentLogic;
namespace luabridge {
    class LuaRef;
};

#include <serenity/ecs/Entity.h>
#include <serenity/ecs/ECSSystemConstructorInfo.h>
#include <serenity/lua/Lua.h>

class ComponentLogic {
    using c_function = void(*)(const ComponentLogic*, const float);
    private:
        LuaCallableFunction<ComponentLogic, const float>   m_Functor;
        void*                                              m_UserPointer   = nullptr;
        void*                                              m_UserPointer1  = nullptr;
        void*                                              m_UserPointer2  = nullptr;
        Entity                                             m_Owner;
    public:
        ComponentLogic(Entity entity) 
            : m_Owner(entity)
        {}
        ComponentLogic(Entity entity, c_function&& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) 
            : m_Owner(entity)
            , m_UserPointer1(Ptr2)
            , m_UserPointer2(Ptr3)
        {
            m_UserPointer  = Ptr1;
            setFunctor(std::move(Functor));
        }
        ComponentLogic(Entity entity, luabridge::LuaRef luaFunction, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_Owner(entity)
            , m_UserPointer1(Ptr2)
            , m_UserPointer2(Ptr3)
        {
            m_UserPointer = Ptr1;
            setFunctor(luaFunction);
        }

        ComponentLogic(const ComponentLogic&)            = delete;
        ComponentLogic& operator=(const ComponentLogic&) = delete;
        ComponentLogic(ComponentLogic&&) noexcept;
        ComponentLogic& operator=(ComponentLogic&&) noexcept;

        ~ComponentLogic() = default;

        [[nodiscard]] inline constexpr Entity getOwner() const noexcept { return m_Owner; }
        void call(const float dt) const noexcept;

        inline void setFunctor(c_function&& functor) noexcept { m_Functor.setFunctor(std::move(functor)); }
        inline void setFunctor(luabridge::LuaRef luaFunction) noexcept {
            ASSERT(!luaFunction.isNil() && luaFunction.isFunction(), __FUNCTION__ << "(luabridge::LuaRef): lua ref is invalid!");
            m_Functor.setFunctor(luaFunction);
        }
        inline void setUserPointer(void* UserPointer) noexcept { m_UserPointer = UserPointer; }
        inline void setUserPointer1(void* UserPointer1) noexcept { m_UserPointer1 = UserPointer1; }
        inline void setUserPointer2(void* UserPointer2) noexcept { m_UserPointer2 = UserPointer2; }
        
        [[nodiscard]] inline constexpr void* getUserPointer() const noexcept { return m_UserPointer; }
        [[nodiscard]] inline constexpr void* getUserPointer1() const noexcept { return m_UserPointer1; }
        [[nodiscard]] inline constexpr void* getUserPointer2() const noexcept { return m_UserPointer2; }
};

class ComponentLogic_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic_System_CI();
        ~ComponentLogic_System_CI() = default;
};

#endif
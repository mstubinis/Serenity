#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

class ComponentLogic2;
namespace luabridge {
    class LuaRef;
};

#include <serenity/ecs/entity/Entity.h>
#include <serenity/lua/Lua.h>
#include <serenity/ecs/components/ComponentBaseClass.h>

class ComponentLogic2 : public ComponentBaseClass<ComponentLogic2> {
    using c_function = void(*)(const ComponentLogic2*, const float);
    private:
        LuaCallableFunction<ComponentLogic2, const float>  m_Functor;
        void*                                              m_UserPointer  = nullptr;
        void*                                              m_UserPointer1 = nullptr;
        void*                                              m_UserPointer2 = nullptr;
        Entity                                             m_Owner;
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
        ComponentLogic2(const ComponentLogic2&)            = delete;
        ComponentLogic2& operator=(const ComponentLogic2&) = delete;
        ComponentLogic2(ComponentLogic2&&) noexcept;
        ComponentLogic2& operator=(ComponentLogic2&&) noexcept;

        ~ComponentLogic2() = default;

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

#endif
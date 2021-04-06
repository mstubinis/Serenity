#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_3_H
#define ENGINE_ECS_COMPONENT_LOGIC_3_H

class ComponentLogic3;
namespace luabridge {
    class LuaRef;
};

#include <serenity/lua/Lua.h>
#include <serenity/ecs/components/ComponentBaseClass.h>

class ComponentLogic3 : public ComponentBaseClass<ComponentLogic3> {
    using CFunction = void(*)(const ComponentLogic3*, const float);
    private:
        LuaCallableFunction<ComponentLogic3, const float>  m_Functor;
        void*                                              m_UserPointer  = nullptr;
        void*                                              m_UserPointer1 = nullptr;
        void*                                              m_UserPointer2 = nullptr;
        Entity                                             m_Owner;
    public:
        ComponentLogic3(Entity entity) 
            : m_Owner{ entity }
        {}
        ComponentLogic3(Entity entity, CFunction&& CFunctor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_Owner       { entity }
            , m_UserPointer { Ptr1 }
            , m_UserPointer1{ Ptr2 }
            , m_UserPointer2{ Ptr3 }
        {
            setFunctor(std::move(CFunctor));
        }
        ComponentLogic3(Entity entity, const CFunction& CFunctor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_Owner       { entity }
            , m_UserPointer { Ptr1 }
            , m_UserPointer1{ Ptr2 }
            , m_UserPointer2{ Ptr3 }
        {
            setFunctor(CFunctor);
        }
        ComponentLogic3(Entity entity, luabridge::LuaRef luaFunction, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_Owner       { entity }
            , m_UserPointer { Ptr1 }
            , m_UserPointer1{ Ptr2 }
            , m_UserPointer2{ Ptr3 }
        {
            setFunctor(luaFunction);
        }
        ComponentLogic3(const ComponentLogic3&)            = delete;
        ComponentLogic3& operator=(const ComponentLogic3&) = delete;
        ComponentLogic3(ComponentLogic3&&) noexcept;
        ComponentLogic3& operator=(ComponentLogic3&&) noexcept;

        [[nodiscard]] inline constexpr Entity getOwner() const noexcept { return m_Owner; }
        void call(const float dt) const noexcept;

        inline void setFunctor(const CFunction& functor) noexcept { m_Functor.setFunctor(functor); }
        inline void setFunctor(CFunction&& functor) noexcept { m_Functor.setFunctor(std::move(functor)); }
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
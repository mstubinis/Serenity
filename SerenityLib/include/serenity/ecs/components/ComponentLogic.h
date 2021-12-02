#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

class ComponentLogic;
namespace luabridge {
    class LuaRef;
};

#include <serenity/lua/Lua.h>
#include <serenity/ecs/components/ComponentBaseClass.h>

class ComponentLogic : public ComponentBaseClass<ComponentLogic> {
    using CFunction = void(*)(const ComponentLogic*, const float);
    private:
        LuaCallableFunction<ComponentLogic, const float>   m_Functor;
        void*                                              m_UserPointer   = nullptr;
        void*                                              m_UserPointer1  = nullptr;
        void*                                              m_UserPointer2  = nullptr;
        Entity                                             m_Owner;
    public:
        ComponentLogic(Entity entity) 
            : m_Owner{ entity }
        {}
        ComponentLogic(Entity entity, CFunction&& CFunctor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_UserPointer{ Ptr1 }
            , m_UserPointer1{ Ptr2 }
            , m_UserPointer2{ Ptr3 }
            , m_Owner{ entity }
        {
            setFunctor(std::move(CFunctor));
        }
        ComponentLogic(Entity entity, const CFunction& CFunctor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_UserPointer{ Ptr1 }
            , m_UserPointer1{ Ptr2 }
            , m_UserPointer2{ Ptr3 }
            , m_Owner{ entity }
        {
            setFunctor(CFunctor);
        }
        ComponentLogic(Entity entity, luabridge::LuaRef luaFunction, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr)
            : m_UserPointer{ Ptr1 }
            , m_UserPointer1{ Ptr2 }
            , m_UserPointer2{ Ptr3 }
            , m_Owner{ entity }
        {
            setFunctor(luaFunction);
        }

        ComponentLogic(const ComponentLogic&)            = delete;
        ComponentLogic& operator=(const ComponentLogic&) = delete;
        ComponentLogic(ComponentLogic&&) noexcept;
        ComponentLogic& operator=(ComponentLogic&&) noexcept;

        [[nodiscard]] inline Entity getOwner() const noexcept { return m_Owner; }
        [[nodiscard]] inline void* getUserPointer() const noexcept { return m_UserPointer; }
        [[nodiscard]] inline void* getUserPointer1() const noexcept { return m_UserPointer1; }
        [[nodiscard]] inline void* getUserPointer2() const noexcept { return m_UserPointer2; }

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
        
};

#endif
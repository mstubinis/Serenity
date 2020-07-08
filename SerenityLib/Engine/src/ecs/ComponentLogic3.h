#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_3_H
#define ENGINE_ECS_COMPONENT_LOGIC_3_H

class ComponentLogic3;
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic3 : public Engine::UserPointer {
    private:
        Entity                                             m_Owner;
        void*                                              m_UserPointer1 = nullptr;
        void*                                              m_UserPointer2 = nullptr;
        LuaCallableFunction<ComponentLogic3, const float>  m_Functor;
    public:
        ComponentLogic3(Entity entity);
        template<typename T> 
        ComponentLogic3(Entity entity, const T& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
            m_Owner        = entity;
            m_UserPointer  = Ptr1;
            m_UserPointer1 = Ptr2;
            m_UserPointer2 = Ptr3;
            setFunctor(Functor);
        }
        ComponentLogic3(const ComponentLogic3& other) = delete;
        ComponentLogic3& operator=(const ComponentLogic3& other) = delete;
        ComponentLogic3(ComponentLogic3&& other) noexcept;
        ComponentLogic3& operator=(ComponentLogic3&& other) noexcept;

        ~ComponentLogic3();

        const Entity getOwner() const;
        void call(const float dt) const;

        void setFunctor(std::function<void(const ComponentLogic3*, const float)> functor);
        void setFunctor(luabridge::LuaRef luaFunction);

        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic3_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic3_System_CI();
        ~ComponentLogic3_System_CI() = default;
};

#endif
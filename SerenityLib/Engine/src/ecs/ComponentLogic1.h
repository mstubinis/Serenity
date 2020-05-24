#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_1_H
#define ENGINE_ECS_COMPONENT_LOGIC_1_H

class ComponentLogic1;
namespace Engine::priv {
    struct ComponentLogic1_UpdateFunction;
    struct ComponentLogic1_EntityAddedToSceneFunction;
    struct ComponentLogic1_ComponentAddedToEntityFunction;
    struct ComponentLogic1_ComponentRemovedFromEntityFunction;
    struct ComponentLogic1_SceneEnteredFunction;
    struct ComponentLogic1_SceneLeftFunction;
};
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic1 : public Engine::UserPointer {
    friend struct Engine::priv::ComponentLogic1_UpdateFunction;
    friend struct Engine::priv::ComponentLogic1_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic1_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentLogic1_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic1_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic1_SceneLeftFunction;
    private:
        Entity m_Owner;

        void*                                                        m_UserPointer1 = nullptr;
        void*                                                        m_UserPointer2 = nullptr;
        LuaCallableUpdateFunction<ComponentLogic1>                   m_Functor;
    public:
        ComponentLogic1(const Entity entity);
        template<typename T> 
        ComponentLogic1(const Entity entity, const T& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
            m_Owner        = entity;
            m_UserPointer  = Ptr1;
            m_UserPointer1 = Ptr2;
            m_UserPointer2 = Ptr3;
            setFunctor(Functor);
        }
        ComponentLogic1(const ComponentLogic1& other) = delete;
        ComponentLogic1& operator=(const ComponentLogic1& other) = delete;
        ComponentLogic1(ComponentLogic1&& other) noexcept;
        ComponentLogic1& operator=(ComponentLogic1&& other) noexcept;

        ~ComponentLogic1();

        const Entity getOwner() const;
        void call(const float dt) const;

        void setFunctor(std::function<void(const ComponentLogic1*, const float)> functor);
        void setFunctor(luabridge::LuaRef luaFunction);

        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic1_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic1_System_CI();
        ~ComponentLogic1_System_CI() = default;
};
#endif
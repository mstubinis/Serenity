#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

class ComponentLogic;
namespace Engine::priv {
    struct ComponentLogic_UpdateFunction;
    struct ComponentLogic_EntityAddedToSceneFunction;
    struct ComponentLogic_ComponentAddedToEntityFunction;
    struct ComponentLogic_ComponentRemovedFromEntityFunction;
    struct ComponentLogic_SceneEnteredFunction;
    struct ComponentLogic_SceneLeftFunction;
};
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic : public Engine::UserPointer {
    friend struct Engine::priv::ComponentLogic_UpdateFunction;
    friend struct Engine::priv::ComponentLogic_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentLogic_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic_SceneLeftFunction;
    private:
        Entity                                             m_Owner;
        void*                                              m_UserPointer1  = nullptr;
        void*                                              m_UserPointer2  = nullptr;
        LuaCallableFunction<ComponentLogic, const float>   m_Functor;
    public:
        ComponentLogic(const Entity entity);
        template<typename T> 
        ComponentLogic(const Entity entity, const T& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
            m_Owner        = entity;
            m_UserPointer  = Ptr1;
            m_UserPointer1 = Ptr2;
            m_UserPointer2 = Ptr3;
            setFunctor(Functor);
        }
        ComponentLogic(const ComponentLogic& other) = delete;
        ComponentLogic& operator=(const ComponentLogic& other) = delete;
        ComponentLogic(ComponentLogic&& other) noexcept;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept;

        ~ComponentLogic();

        const Entity getOwner() const;
        void call(const float dt) const;

        void setFunctor(std::function<void(const ComponentLogic*, const float)> functor);
        void setFunctor(luabridge::LuaRef luaFunction);

        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic_System_CI();
        ~ComponentLogic_System_CI() = default;
};

#endif
#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

class ComponentLogic2;
namespace Engine::priv {
    struct ComponentLogic2_UpdateFunction;
    struct ComponentLogic2_EntityAddedToSceneFunction;
    struct ComponentLogic2_ComponentAddedToEntityFunction;
    struct ComponentLogic2_ComponentRemovedFromEntityFunction;
    struct ComponentLogic2_SceneEnteredFunction;
    struct ComponentLogic2_SceneLeftFunction;
};
namespace luabridge {
    class LuaRef;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/lua/Lua.h>

class ComponentLogic2 : public Engine::UserPointer {
    friend struct Engine::priv::ComponentLogic2_UpdateFunction;
    friend struct Engine::priv::ComponentLogic2_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic2_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentLogic2_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic2_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic2_SceneLeftFunction;
    private:
        Entity                                             m_Owner;
        void*                                              m_UserPointer1 = nullptr;
        void*                                              m_UserPointer2 = nullptr;
        LuaCallableFunction<ComponentLogic2, const float>  m_Functor;
    public:
        ComponentLogic2(Entity entity);
        template<typename T> 
        ComponentLogic2(Entity entity, const T& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
            m_Owner        = entity;
            m_UserPointer  = Ptr1;
            m_UserPointer1 = Ptr2;
            m_UserPointer2 = Ptr3;
            setFunctor(Functor);
        }
        ComponentLogic2(const ComponentLogic2& other) = delete;
        ComponentLogic2& operator=(const ComponentLogic2& other) = delete;
        ComponentLogic2(ComponentLogic2&& other) noexcept;
        ComponentLogic2& operator=(ComponentLogic2&& other) noexcept;

        ~ComponentLogic2();

        const Entity getOwner() const;
        void call(const float dt) const;

        void setFunctor(std::function<void(const ComponentLogic2*, const float)> functor);
        void setFunctor(luabridge::LuaRef luaFunction);

        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic2_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic2_System_CI();
        ~ComponentLogic2_System_CI() = default;
};

#endif
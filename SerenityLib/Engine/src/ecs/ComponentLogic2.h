#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentLogic2;
namespace Engine::priv {
    struct ComponentLogic2_UpdateFunction;
    struct ComponentLogic2_EntityAddedToSceneFunction;
    struct ComponentLogic2_ComponentAddedToEntityFunction;
    struct ComponentLogic2_ComponentRemovedFromEntityFunction;
    struct ComponentLogic2_SceneEnteredFunction;
    struct ComponentLogic2_SceneLeftFunction;
};

class ComponentLogic2 {
    friend struct Engine::priv::ComponentLogic2_UpdateFunction;
    friend struct Engine::priv::ComponentLogic2_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic2_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentLogic2_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic2_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic2_SceneLeftFunction;

    private:
        Entity m_Owner;

        void*                                                        m_UserPointer  = nullptr;
        void*                                                        m_UserPointer1 = nullptr;
        void*                                                        m_UserPointer2 = nullptr;
        std::function<void(const ComponentLogic2*, const float)>     m_Functor      = [](const ComponentLogic2*, const float) {};
    public:
        ComponentLogic2(const Entity& entity);
        template<typename T> 
        ComponentLogic2(const Entity& entity, const T& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
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

        void call(const float dt) const;

        template<typename T> void setFunctor(const T& functor) { 
            m_Functor = std::bind<void>(std::move(functor), std::placeholders::_1, std::placeholders::_2);
        }

        void setUserPointer(void* UserPointer);
        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer() const;
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic2_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic2_System_CI();
        ~ComponentLogic2_System_CI() = default;
};

#endif
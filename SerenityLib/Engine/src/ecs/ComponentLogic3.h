#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_3_H
#define ENGINE_ECS_COMPONENT_LOGIC_3_H

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentLogic3;
namespace Engine::priv {
    struct ComponentLogic3_UpdateFunction;
    struct ComponentLogic3_EntityAddedToSceneFunction;
    struct ComponentLogic3_ComponentAddedToEntityFunction;
    struct ComponentLogic3_ComponentRemovedFromEntityFunction;
    struct ComponentLogic3_SceneEnteredFunction;
    struct ComponentLogic3_SceneLeftFunction;
};

class ComponentLogic3 : public Engine::UserPointer {
    friend struct Engine::priv::ComponentLogic3_UpdateFunction;
    friend struct Engine::priv::ComponentLogic3_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic3_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentLogic3_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic3_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic3_SceneLeftFunction;
    private:
        Entity m_Owner;

        void*                                                        m_UserPointer1 = nullptr;
        void*                                                        m_UserPointer2 = nullptr;
        std::function<void(const ComponentLogic3*, const float)>     m_Functor      = [](const ComponentLogic3*, const float) {};
    public:
        ComponentLogic3(const Entity entity);
        template<typename T> 
        ComponentLogic3(const Entity entity, const T& Functor, void* Ptr1 = nullptr, void* Ptr2 = nullptr, void* Ptr3 = nullptr) {
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

        template<typename T> void setFunctor(const T& functor) { 
            m_Functor = std::bind<void>(std::move(functor), std::placeholders::_1, std::placeholders::_2);
        }
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
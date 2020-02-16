#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentLogic;
namespace Engine {
    namespace priv {
        struct ComponentLogic_UpdateFunction;
        struct ComponentLogic_EntityAddedToSceneFunction;
        struct ComponentLogic_ComponentAddedToEntityFunction;
        struct ComponentLogic_SceneEnteredFunction;
        struct ComponentLogic_SceneLeftFunction;
    };
};

class ComponentLogic : public ComponentBaseClass {
    friend struct Engine::priv::ComponentLogic_UpdateFunction;
    friend struct Engine::priv::ComponentLogic_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic_SceneLeftFunction;

    private:
        void*                                m_UserPointer;
        void*                                m_UserPointer1;
        void*                                m_UserPointer2;
        std::function<void(const float&)>    m_Functor;
    public:
        ComponentLogic(const Entity& entity);
        template<typename T> ComponentLogic(const Entity& entity, const T& Functor, void* UserPointer = 0, void* UserPointer1 = 0, void* UserPointer2 = 0) : ComponentBaseClass(entity) {
            m_UserPointer  = UserPointer;
            m_UserPointer1 = UserPointer1;
            m_UserPointer2 = UserPointer2;
            setFunctor(Functor);
        }
        ComponentLogic(const ComponentLogic& other) = delete;
        ComponentLogic& operator=(const ComponentLogic& other) = delete;
        ComponentLogic(ComponentLogic&& other) noexcept;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept;

        ~ComponentLogic();

        void call(const float& dt);

        template<typename T> void setFunctor(const T& functor) { 
            m_Functor = std::bind<void>(std::move(functor), this, std::placeholders::_1); 
        }
        void setUserPointer(void* UserPointer);
        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer() const;
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic_System_CI();
        ~ComponentLogic_System_CI() = default;
};

#endif
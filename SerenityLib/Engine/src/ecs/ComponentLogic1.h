#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_1_H
#define ENGINE_ECS_COMPONENT_LOGIC_1_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentLogic1;
namespace Engine {
    namespace priv {
        struct ComponentLogic1_UpdateFunction;
        struct ComponentLogic1_EntityAddedToSceneFunction;
        struct ComponentLogic1_ComponentAddedToEntityFunction;
        struct ComponentLogic1_SceneEnteredFunction;
        struct ComponentLogic1_SceneLeftFunction;
    };
};

class ComponentLogic1 : public ComponentBaseClass {
    friend struct Engine::priv::ComponentLogic1_UpdateFunction;
    friend struct Engine::priv::ComponentLogic1_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentLogic1_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentLogic1_SceneEnteredFunction;
    friend struct Engine::priv::ComponentLogic1_SceneLeftFunction;
    private:
        void*                                m_UserPointer;
        void*                                m_UserPointer1;
        void*                                m_UserPointer2;
        std::function<void(const float&)>    m_Functor;
    public:
        ComponentLogic1(const Entity& entity);
        template<typename T> ComponentLogic1(const Entity& entity, const T& Functor, void* UserPointer = 0, void* UserPointer1 = 0, void* UserPointer2 = 0) : ComponentBaseClass(entity) {
            m_UserPointer  = UserPointer;
            m_UserPointer1 = UserPointer1;
            m_UserPointer2 = UserPointer2;
            setFunctor(Functor);
        }
        ComponentLogic1(const ComponentLogic1& other) = delete;
        ComponentLogic1& operator=(const ComponentLogic1& other) = delete;
        ComponentLogic1(ComponentLogic1&& other) noexcept;
        ComponentLogic1& operator=(ComponentLogic1&& other) noexcept;

        ~ComponentLogic1();

        void call(const float& dt) const;

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

class ComponentLogic1_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentLogic1_System_CI();
        ~ComponentLogic1_System_CI() = default;
};
#endif
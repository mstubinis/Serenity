#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentLogic2;
namespace Engine {
    namespace epriv {
        struct ComponentLogic2_UpdateFunction;
        struct ComponentLogic2_EntityAddedToSceneFunction;
        struct ComponentLogic2_ComponentAddedToEntityFunction;
        struct ComponentLogic2_SceneEnteredFunction;
        struct ComponentLogic2_SceneLeftFunction;
        struct ComponentLogic2_EmptyFunctor final { void operator()(ComponentLogic2& _component, const double& dt) const {} };
    };
};

class ComponentLogic2 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic2_UpdateFunction;
    friend struct Engine::epriv::ComponentLogic2_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic2_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic2_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic2_SceneLeftFunction;

    private:
        void*                                m_UserPointer;
        void*                                m_UserPointer1;
        void*                                m_UserPointer2;
        std::function<void(const double&)>   m_Functor;
    public:
        ComponentLogic2(const Entity& entity);
        template<typename T> ComponentLogic2(const Entity& entity, const T& Functor, void* UserPointer = 0, void* UserPointer1 = 0, void* UserPointer2 = 0) : ComponentBaseClass(entity) {
            m_UserPointer  = UserPointer;
            m_UserPointer1 = UserPointer1;
            m_UserPointer2 = UserPointer2;
            setFunctor(Functor);
        }
        ComponentLogic2(const ComponentLogic2& other);
        ComponentLogic2& operator=(const ComponentLogic2& other);
        ComponentLogic2(ComponentLogic2&& other) noexcept;
        ComponentLogic2& operator=(ComponentLogic2&& other) noexcept;

        ~ComponentLogic2();

        void call(const double& dt);

        template<typename T> void setFunctor(const T& functor) { 
            m_Functor = std::bind<void>(std::move(functor), *this, std::placeholders::_1);
        }

        void setUserPointer(void* UserPointer);
        void setUserPointer1(void* UserPointer1);
        void setUserPointer2(void* UserPointer2);
        
        void* getUserPointer() const;
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic2_System_CI : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic2_System_CI();
        ~ComponentLogic2_System_CI() = default;
};

#endif
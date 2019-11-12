#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_3_H
#define ENGINE_ECS_COMPONENT_LOGIC_3_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentLogic3;
namespace Engine {
    namespace epriv {
        struct ComponentLogic3_UpdateFunction;
        struct ComponentLogic3_EntityAddedToSceneFunction;
        struct ComponentLogic3_ComponentAddedToEntityFunction;
        struct ComponentLogic3_SceneEnteredFunction;
        struct ComponentLogic3_SceneLeftFunction;
        struct ComponentLogic3_EmptyFunctor final { void operator()(ComponentLogic3& _component, const double& dt) const {} };
    };
};

class ComponentLogic3 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic3_UpdateFunction;
    friend struct Engine::epriv::ComponentLogic3_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic3_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic3_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic3_SceneLeftFunction;
    private:
        void*                                m_UserPointer;
        void*                                m_UserPointer1;
        void*                                m_UserPointer2;
        std::function<void(const double&)>   m_Functor;
    public:
        ComponentLogic3(const Entity& entity);
        template<typename T> ComponentLogic3(const Entity& entity, const T& Functor, void* UserPointer = 0, void* UserPointer1 = 0, void* UserPointer2 = 0) : ComponentBaseClass(entity) {
            m_UserPointer  = UserPointer;
            m_UserPointer1 = UserPointer1;
            m_UserPointer2 = UserPointer2;
            setFunctor(Functor);
        }
        ComponentLogic3(const ComponentLogic3& other);
        ComponentLogic3& operator=(const ComponentLogic3& other);
        ComponentLogic3(ComponentLogic3&& other) noexcept;
        ComponentLogic3& operator=(ComponentLogic3&& other) noexcept;

        ~ComponentLogic3();

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

class ComponentLogic3_System_CI : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic3_System_CI();
        ~ComponentLogic3_System_CI() = default;
};

#endif
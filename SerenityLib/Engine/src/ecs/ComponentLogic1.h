#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_1_H
#define ENGINE_ECS_COMPONENT_LOGIC_1_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <functional>

class ComponentLogic1;
namespace Engine {
    namespace epriv {
        struct ComponentLogic1_UpdateFunction;
        struct ComponentLogic1_EntityAddedToSceneFunction;
        struct ComponentLogic1_ComponentAddedToEntityFunction;
        struct ComponentLogic1_SceneEnteredFunction;
        struct ComponentLogic1_SceneLeftFunction;
        struct ComponentLogic1_EmptyFunctor final { void operator()(ComponentLogic1& _component, const double& dt) const {} };
    };
};

class ComponentLogic1 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic1_UpdateFunction;
    friend struct Engine::epriv::ComponentLogic1_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic1_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic1_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic1_SceneLeftFunction;
    private:
        void*                                _userPtr;
        void*                                _userPtr1;
        void*                                _userPtr2;
        std::function<void(const double&)>   _functor;
    public:
        ComponentLogic1(const Entity& _e);
        template<typename T> ComponentLogic1(const Entity& _e, const T& functor, void* ptr = 0, void* ptr1 = 0, void* ptr2 = 0) : ComponentBaseClass(_e) {
            _userPtr = ptr;
            _userPtr1 = ptr1;
            _userPtr2 = ptr2;
            setFunctor(functor);
        }
        ComponentLogic1(const ComponentLogic1& other);
        ComponentLogic1& operator=(const ComponentLogic1& other);
        ComponentLogic1(ComponentLogic1&& other) noexcept;
        ComponentLogic1& operator=(ComponentLogic1&& other) noexcept;

        ~ComponentLogic1();

        void call(const double& dt);

        template<typename T> void setFunctor(const T& functor) { 
            _functor = std::bind<void>(std::move(functor), *this, std::placeholders::_1);
        }

        void setUserPointer(void* ptr);
        void setUserPointer1(void* ptr);
        void setUserPointer2(void* ptr);
        
        void* getUserPointer() const;
        void* getUserPointer1() const;
        void* getUserPointer2() const;
};

class ComponentLogic1_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic1_System();
        ~ComponentLogic1_System() = default;
};
#endif
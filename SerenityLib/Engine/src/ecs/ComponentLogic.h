#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <functional>

class ComponentLogic;
namespace Engine {
    namespace epriv {
        struct ComponentLogic_UpdateFunction;
        struct ComponentLogic_EntityAddedToSceneFunction;
        struct ComponentLogic_ComponentAddedToEntityFunction;
        struct ComponentLogic_SceneEnteredFunction;
        struct ComponentLogic_SceneLeftFunction;
        struct ComponentLogic_EmptyFunctor final { void operator()(ComponentLogic& _component, const double& dt) const {}};
    };
};

class ComponentLogic : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic_UpdateFunction;
    friend struct Engine::epriv::ComponentLogic_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic_SceneLeftFunction;

    private:
        void*                                _userPtr;
        void*                                _userPtr1;
        void*                                _userPtr2;
        std::function<void(const double&)>   _functor;
    public:
        ComponentLogic(const Entity& _e);
        template<typename T> ComponentLogic(const Entity& _e, const T& functor, void* ptr = 0, void* ptr1 = 0, void* ptr2 = 0) : ComponentBaseClass(_e) {
            _userPtr  = ptr;
            _userPtr1 = ptr1;
            _userPtr2 = ptr2;
            setFunctor(functor);
        }
        ComponentLogic(const ComponentLogic& other);
        ComponentLogic& operator=(const ComponentLogic& other);
        ComponentLogic(ComponentLogic&& other) noexcept;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept;

        ~ComponentLogic();

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

class ComponentLogic_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic_System();
        ~ComponentLogic_System() = default;
};

#endif
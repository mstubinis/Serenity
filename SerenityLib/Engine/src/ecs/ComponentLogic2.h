#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <functional>

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
        void*                                _userPtr;
        void*                                _userPtr1;
        void*                                _userPtr2;
        std::function<void(const double&)>   _functor;
    public:
        ComponentLogic2(const Entity& _e);
        template<typename T> ComponentLogic2(const Entity& _e, const T& functor, void* ptr = 0, void* ptr1 = 0, void* ptr2 = 0) : ComponentBaseClass(_e) {
            _userPtr = ptr;
            _userPtr1 = ptr1;
            _userPtr2 = ptr2;
            setFunctor(functor);
        }
        ComponentLogic2(const ComponentLogic2& other);
        ComponentLogic2& operator=(const ComponentLogic2& other);
        ComponentLogic2(ComponentLogic2&& other) noexcept;
        ComponentLogic2& operator=(ComponentLogic2&& other) noexcept;

        ~ComponentLogic2();

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

class ComponentLogic2_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic2_System();
        ~ComponentLogic2_System() = default;
};

#endif
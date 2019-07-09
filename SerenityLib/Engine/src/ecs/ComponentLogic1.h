#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_1_H
#define ENGINE_ECS_COMPONENT_LOGIC_1_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

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
        void*                               _userPtr;
        boost::function<void(const float&)> _functor;
    public:
        ComponentLogic1(const Entity& _e) : ComponentBaseClass(_e) {
			_userPtr = nullptr;
			setFunctor(Engine::epriv::ComponentLogic1_EmptyFunctor()); 
		}
        template<typename T> ComponentLogic1(const Entity& _e, const T& functor) : ComponentBaseClass(_e) {
			_userPtr = nullptr;
			setFunctor(functor); 
		}
        template<typename T,typename V> ComponentLogic1(const Entity& _e, const T& functor, V* userPointer) : ComponentBaseClass(_e) {
			_userPtr = userPointer;
			setFunctor(functor); 
		}

        ComponentLogic1(const ComponentLogic1& other);
        ComponentLogic1& operator=(const ComponentLogic1& other);
        ComponentLogic1(ComponentLogic1&& other) noexcept;
        ComponentLogic1& operator=(ComponentLogic1&& other) noexcept;

        ~ComponentLogic1();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const double& dt);
        void* getUserPointer() { return _userPtr; }
};

class ComponentLogic1_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic1_System();
        ~ComponentLogic1_System() = default;
};
#endif
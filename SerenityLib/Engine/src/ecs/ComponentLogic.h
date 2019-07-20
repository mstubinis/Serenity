#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

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
        boost::function<void(const double&)> _functor;
    public:
        ComponentLogic(const Entity& _e) : ComponentBaseClass(_e){ 
			_userPtr = nullptr;
			setFunctor(Engine::epriv::ComponentLogic_EmptyFunctor()); 
		}
        template<typename T> ComponentLogic(const Entity& _e, const T& functor) : ComponentBaseClass(_e) {
			_userPtr = nullptr;
			setFunctor(functor); 
		}
        template<typename T,typename V> ComponentLogic(const Entity& _e, const T& functor, V* userPointer) : ComponentBaseClass(_e) {
			_userPtr = userPointer;
			setFunctor(functor); 
		}

        ComponentLogic(const ComponentLogic& other);
        ComponentLogic& operator=(const ComponentLogic& other);
        ComponentLogic(ComponentLogic&& other) noexcept;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept;

        ~ComponentLogic();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const double& dt);
        void* getUserPointer() { return _userPtr; }
};

class ComponentLogic_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic_System();
        ~ComponentLogic_System() = default;
};

#endif
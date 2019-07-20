#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_3_H
#define ENGINE_ECS_COMPONENT_LOGIC_3_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

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
        void*                                _userPtr;
        boost::function<void(const double&)> _functor;
    public:
        ComponentLogic3(const Entity& _e) : ComponentBaseClass(_e) { 
			_userPtr = nullptr;
			setFunctor(Engine::epriv::ComponentLogic3_EmptyFunctor()); 
		}
        template<typename T> ComponentLogic3(const Entity& _e, const T& functor) : ComponentBaseClass(_e) { 
			_userPtr = nullptr;
			setFunctor(functor); 
		}
        template<typename T,typename V> ComponentLogic3(const Entity& _e, const T& functor, V* userPointer) : ComponentBaseClass(_e) { 
			_userPtr = userPointer;
			setFunctor(functor); 
		}

        ComponentLogic3(const ComponentLogic3& other);
        ComponentLogic3& operator=(const ComponentLogic3& other);
        ComponentLogic3(ComponentLogic3&& other) noexcept;
        ComponentLogic3& operator=(ComponentLogic3&& other) noexcept;

        ~ComponentLogic3();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const double& dt);
        void* getUserPointer() { return _userPtr; }
};

class ComponentLogic3_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic3_System();
        ~ComponentLogic3_System() = default;
};

#endif
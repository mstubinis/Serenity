#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_1_H
#define ENGINE_ECS_COMPONENT_LOGIC_1_H

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

class ComponentLogic1;
namespace Engine {
    namespace epriv {
        struct ComponentLogic1UpdateFunction;
        struct ComponentLogic1EntityAddedToSceneFunction;
        struct ComponentLogic1ComponentAddedToEntityFunction;
        struct ComponentLogic1SceneEnteredFunction;
        struct ComponentLogic1SceneLeftFunction;
        struct ComponentLogic1EmptyFunctor final { void operator()(ComponentLogic1& _component, const float& dt) const {} };
    };
};

class ComponentLogic1 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic1UpdateFunction;
    friend struct Engine::epriv::ComponentLogic1ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic1EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic1SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic1SceneLeftFunction;

    private:
        boost::function<void(const float&)> _functor;
        void*                               _userPtr;
    public:
        ComponentLogic1(Entity&) { setFunctor(Engine::epriv::ComponentLogic1EmptyFunctor()); _userPtr = nullptr; }
        template<typename T> ComponentLogic1(Entity&, const T& functor) { setFunctor(functor); _userPtr = nullptr; }
        template<typename T> ComponentLogic1(Entity&, const T& functor, void* userPointer) { setFunctor(functor); _userPtr = userPointer; }

        ComponentLogic1(const ComponentLogic1& other) = default;
        ComponentLogic1& operator=(const ComponentLogic1& other) = default;
        ComponentLogic1(ComponentLogic1&& other) noexcept = default;
        ComponentLogic1& operator=(ComponentLogic1&& other) noexcept = default;

        ~ComponentLogic1();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const float& dt);
        const void* getUserPointer() { return _userPtr; }
};

class ComponentLogic1System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic1System();
        ~ComponentLogic1System() = default;
};
#endif
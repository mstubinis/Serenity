#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

class ComponentLogic2;
namespace Engine {
    namespace epriv {
        struct ComponentLogic2UpdateFunction;
        struct ComponentLogic2EntityAddedToSceneFunction;
        struct ComponentLogic2ComponentAddedToEntityFunction;
        struct ComponentLogic2SceneEnteredFunction;
        struct ComponentLogic2SceneLeftFunction;
        struct ComponentLogic2EmptyFunctor final { void operator()(ComponentLogic2& _component, const float& dt) const {} };
    };
};

class ComponentLogic2 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic2UpdateFunction;
    friend struct Engine::epriv::ComponentLogic2ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic2EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic2SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic2SceneLeftFunction;

    private:
        void*                               _userPtr;
        boost::function<void(const float&)> _functor;
    public:
        ComponentLogic2(Entity& _e) : ComponentBaseClass(_e) { _userPtr = nullptr; setFunctor(Engine::epriv::ComponentLogic2EmptyFunctor()); }
        template<typename T> ComponentLogic2(Entity& _e, const T& functor) : ComponentBaseClass(_e) { _userPtr = nullptr; setFunctor(functor); }
        template<typename T,typename V> ComponentLogic2(Entity& _e, const T& functor, V* userPointer) : ComponentBaseClass(_e) { _userPtr = userPointer; setFunctor(functor); }

        ComponentLogic2(const ComponentLogic2& other);
        ComponentLogic2& operator=(const ComponentLogic2& other);
        ComponentLogic2(ComponentLogic2&& other) noexcept;
        ComponentLogic2& operator=(ComponentLogic2&& other) noexcept;

        ~ComponentLogic2();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const float& dt);
        void* getUserPointer() { return _userPtr; }
};

class ComponentLogic2System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic2System();
        ~ComponentLogic2System() = default;
};

#endif
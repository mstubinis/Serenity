#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

class ComponentLogic;
namespace Engine {
    namespace epriv {
        struct ComponentLogicUpdateFunction;
        struct ComponentLogicEntityAddedToSceneFunction;
        struct ComponentLogicComponentAddedToEntityFunction;
        struct ComponentLogicSceneEnteredFunction;
        struct ComponentLogicSceneLeftFunction;
        struct ComponentLogicEmptyFunctor final { void operator()(ComponentLogic& _component, const float& dt) const {}};
    };
};

class ComponentLogic : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogicUpdateFunction;
    friend struct Engine::epriv::ComponentLogicComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogicEntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogicSceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogicSceneLeftFunction;

    private:
        void*                               _userPtr;
        boost::function<void(const float&)> _functor;
    public:
        ComponentLogic(Entity& _e) : ComponentBaseClass(_e){ _userPtr = nullptr; setFunctor(Engine::epriv::ComponentLogicEmptyFunctor()); }
        template<typename T> ComponentLogic(Entity& _e, const T& functor) : ComponentBaseClass(_e) { _userPtr = nullptr; setFunctor(functor); }
        template<typename T,typename V> ComponentLogic(Entity& _e, const T& functor, V* userPointer) : ComponentBaseClass(_e) { _userPtr = userPointer; setFunctor(functor); }

        ComponentLogic(const ComponentLogic& other);
        ComponentLogic& operator=(const ComponentLogic& other);
        ComponentLogic(ComponentLogic&& other) noexcept;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept;

        ~ComponentLogic();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const float& dt);
        void* getUserPointer() { return _userPtr; }
};

class ComponentLogicSystem : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogicSystem();
        ~ComponentLogicSystem() = default;
};

#endif
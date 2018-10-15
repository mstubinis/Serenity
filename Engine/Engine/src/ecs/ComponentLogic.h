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
        boost::function<void(const float&)> _functor;
        void*                               _userPtr;
    public:
        ComponentLogic(Entity&) { setFunctor(Engine::epriv::ComponentLogicEmptyFunctor()); _userPtr = nullptr; }
        template<typename T> ComponentLogic(Entity&, const T& functor) { setFunctor(functor); _userPtr = nullptr; }
        template<typename T> ComponentLogic(Entity&, const T& functor, void* userPointer) { setFunctor(functor); _userPtr = userPointer; }

        ComponentLogic(const ComponentLogic& other) = default;
        ComponentLogic& operator=(const ComponentLogic& other) = default;
        ComponentLogic(ComponentLogic&& other) noexcept = default;
        ComponentLogic& operator=(ComponentLogic&& other) noexcept = default;

        ~ComponentLogic();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const float& dt);
        const void* getUserPointer() { return _userPtr; }
};

class ComponentLogicSystem : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogicSystem();
        ~ComponentLogicSystem() = default;
};

#endif
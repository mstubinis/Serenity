#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_3_H
#define ENGINE_ECS_COMPONENT_LOGIC_3_H

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

class ComponentLogic3;
namespace Engine {
    namespace epriv {
        struct ComponentLogic3UpdateFunction;
        struct ComponentLogic3EntityAddedToSceneFunction;
        struct ComponentLogic3ComponentAddedToEntityFunction;
        struct ComponentLogic3SceneEnteredFunction;
        struct ComponentLogic3SceneLeftFunction;
        struct ComponentLogic3EmptyFunctor final { void operator()(ComponentLogic3& _component, const float& dt) const {} };
    };
};

class ComponentLogic3 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic3UpdateFunction;
    friend struct Engine::epriv::ComponentLogic3ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic3EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic3SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic3SceneLeftFunction;

    private:
        boost::function<void(const float&)> _functor;
        void*                               _userPtr;
    public:
        ComponentLogic3(Entity&) { setFunctor(Engine::epriv::ComponentLogic3EmptyFunctor()); _userPtr = nullptr; }
        template<typename T> ComponentLogic3(Entity&, const T& functor) { setFunctor(functor); _userPtr = nullptr; }
        template<typename T> ComponentLogic3(Entity&, const T& functor, void* userPointer) { setFunctor(functor); _userPtr = userPointer; }

        ComponentLogic3(const ComponentLogic3& other) = default;
        ComponentLogic3& operator=(const ComponentLogic3& other) = default;
        ComponentLogic3(ComponentLogic3&& other) noexcept = default;
        ComponentLogic3& operator=(ComponentLogic3&& other) noexcept = default;

        ~ComponentLogic3();

        template<typename T> void setFunctor(const T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        template<typename T> void setUserPointer(T* ptr) { _userPtr = ptr; }
        void call(const float& dt);
        const void* getUserPointer() { return _userPtr; }
};

class ComponentLogic3System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic3System();
        ~ComponentLogic3System() = default;
};

#endif
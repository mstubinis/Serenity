#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_2_H
#define ENGINE_ECS_COMPONENT_LOGIC_2_H

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace Engine {
    namespace epriv {
        struct ComponentLogic2UpdateFunction;
        struct ComponentLogic2EntityAddedToSceneFunction;
        struct ComponentLogic2ComponentAddedToEntityFunction;
        struct ComponentLogic2SceneEnteredFunction;
        struct ComponentLogic2SceneLeftFunction;
    };
};

class ComponentLogic2 : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentLogic2UpdateFunction;
    friend struct Engine::epriv::ComponentLogic2ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentLogic2EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentLogic2SceneEnteredFunction;
    friend struct Engine::epriv::ComponentLogic2SceneLeftFunction;

    private:
        boost::function<void(const float&)> _functor;
    public:
        ComponentLogic2(Entity&);

        ComponentLogic2(const ComponentLogic2& other) = default;
        ComponentLogic2& operator=(const ComponentLogic2& other) = default;
        ComponentLogic2(ComponentLogic2&& other) noexcept = default;
        ComponentLogic2& operator=(ComponentLogic2&& other) noexcept = default;

        ~ComponentLogic2();

        template<typename T> void setFunctor(T& functor) { _functor = boost::bind<void>(functor, *this, _1); }
        void call(const float& dt);
};

class ComponentLogic2System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogic2System();
        ~ComponentLogic2System() = default;
};

#endif
#pragma once
#ifndef ENGINE_ECS_COMPONENT_LOGIC_H
#define ENGINE_ECS_COMPONENT_LOGIC_H

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace Engine {
    namespace epriv {
        struct ComponentLogicUpdateFunction;
        struct ComponentLogicEntityAddedToSceneFunction;
        struct ComponentLogicComponentAddedToEntityFunction;
        struct ComponentLogicSceneEnteredFunction;
        struct ComponentLogicSceneLeftFunction;
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
    public:
        ComponentLogic(Entity&);
        ~ComponentLogic();

        template<typename T> void setFunctor(T& functor) { _functor = boost::bind<void>(functor, this, _1); }
        void call(const float& dt);
};

class ComponentLogicSystem : public Engine::epriv::ECSSystemCI {
    public:
        ComponentLogicSystem();
        ~ComponentLogicSystem() = default;
};

#endif
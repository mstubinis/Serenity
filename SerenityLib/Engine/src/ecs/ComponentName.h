#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H_INCLUDE_GUARD
#define ENGINE_ECS_COMPONENT_NAME_H_INCLUDE_GUARD

#include <ecs/ComponentBaseClass.h>

namespace Engine {
    namespace epriv {
        struct ComponentName_UpdateFunction;
        struct ComponentName_EntityAddedToSceneFunction;
        struct ComponentName_ComponentAddedToEntityFunction;
        struct ComponentName_SceneEnteredFunction;
        struct ComponentName_SceneLeftFunction;
    };
};

class ComponentName : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentName_UpdateFunction;
    friend struct Engine::epriv::ComponentName_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentName_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentName_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentName_SceneLeftFunction;
    private:
        std::string _data;
    public:
        ComponentName(const Entity&);
        ComponentName(const Entity&, const std::string& name);
        ComponentName(const Entity&, const char* name);

        ComponentName(const ComponentName& other)                = delete;
        ComponentName& operator=(const ComponentName& other)     = delete;
        ComponentName(ComponentName&& other) noexcept            = default;
        ComponentName& operator=(ComponentName&& other) noexcept = default;

        const std::string& name() const;
        const size_t size() const;
        const bool empty() const;

        ~ComponentName();
};

class ComponentName_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentName_System();
        ~ComponentName_System() = default;
};

#endif
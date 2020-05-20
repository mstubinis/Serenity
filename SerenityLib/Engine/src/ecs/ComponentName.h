#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H_INCLUDE_GUARD
#define ENGINE_ECS_COMPONENT_NAME_H_INCLUDE_GUARD

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>

namespace Engine::priv {
    struct ComponentName_UpdateFunction;
    struct ComponentName_EntityAddedToSceneFunction;
    struct ComponentName_ComponentAddedToEntityFunction;
    struct ComponentName_ComponentRemovedFromEntityFunction;
    struct ComponentName_SceneEnteredFunction;
    struct ComponentName_SceneLeftFunction;
};
class ComponentName {
    friend struct Engine::priv::ComponentName_UpdateFunction;
    friend struct Engine::priv::ComponentName_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentName_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentName_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentName_SceneEnteredFunction;
    friend struct Engine::priv::ComponentName_SceneLeftFunction;
    private:
        Entity m_Owner;
        std::string m_Data;
    public:
        ComponentName(const Entity entity);
        ComponentName(const Entity entity, const std::string& name);
        ComponentName(const Entity entity, const char* name);

        ComponentName(const ComponentName& other)                = delete;
        ComponentName& operator=(const ComponentName& other)     = delete;
        ComponentName(ComponentName&& other) noexcept            = default;
        ComponentName& operator=(ComponentName&& other) noexcept = default;

        const std::string& name() const;
        size_t size() const;
        bool empty() const;

        ~ComponentName();
};
class ComponentName_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentName_System_CI();
        ~ComponentName_System_CI() = default;
};

#endif
#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H_INCLUDE_GUARD
#define ENGINE_ECS_COMPONENT_NAME_H_INCLUDE_GUARD

#include "core/engine/Engine_Physics.h"
#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <string>

class ComponentName : public ComponentBaseClass {
    private:
        std::string _data;
    public:
        ComponentName(Entity&);
        ComponentName(Entity&, std::string&);
        ComponentName(Entity&, const char*);

        ComponentName& operator=(const ComponentName& other) = delete;
        ComponentName(const ComponentName& other) = delete;
        ComponentName(ComponentName&& other) noexcept = default;
        ComponentName& operator=(ComponentName&& other) noexcept = default;

        const std::string& name();
        const uint size();

        ~ComponentName();
};

class ComponentNameSystem : public Engine::epriv::ECSSystemCI {
    public:
        ComponentNameSystem();
        ~ComponentNameSystem() = default;
};

#endif
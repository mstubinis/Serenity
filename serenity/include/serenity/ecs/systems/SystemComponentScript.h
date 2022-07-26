#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_SCRIPT_H
#define ENGINE_ECS_SYSTEM_COMPONENT_SCRIPT_H

class  ComponentScript;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemComponentScript final : public SystemCRTP<SystemComponentScript, ComponentScript> {
    friend class  ComponentScript;
    public:
        SystemComponentScript(Engine::priv::ECS&);
};

#endif
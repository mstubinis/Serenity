#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_H

class  ComponentBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemComponentBody final : public SystemCRTP<SystemComponentBody, ComponentBody> {
    friend class  ComponentBody;
    public:
        SystemComponentBody(Engine::priv::ECS& ecs);
};

#endif
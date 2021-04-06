#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_RIGID_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_RIGID_H

class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemComponentRigidBody final : public SystemCRTP<SystemComponentRigidBody, ComponentRigidBody> {
    friend class  ComponentRigidBody;
public:
    SystemComponentRigidBody(Engine::priv::ECS& ecs);
};

#endif
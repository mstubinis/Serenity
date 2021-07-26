#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_RIGID_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_RIGID_H

class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

//adds and removes rigid bodies on scene change, and recalculates mass upon adding entities to scenes, and adds the body to the scene when the entity enters the scene upon creation
class SystemComponentRigidBody final : public SystemCRTP<SystemComponentRigidBody, ComponentRigidBody> {
    friend class  ComponentRigidBody;
    public:
        SystemComponentRigidBody(Engine::priv::ECS& ecs);
};

#endif
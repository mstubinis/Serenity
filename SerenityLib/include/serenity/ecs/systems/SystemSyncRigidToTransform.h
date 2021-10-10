#pragma once
#ifndef ENGINE_ECS_SYSTEM_SYNC_RIGID_TO_TRANSFORM_H
#define ENGINE_ECS_SYSTEM_SYNC_RIGID_TO_TRANSFORM_H

class  ComponentTransform;
class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

//syncs bullet rigid body and collision shape to world matrices
class SystemSyncRigidToTransform final : public SystemCRTP<SystemSyncRigidToTransform, ComponentTransform, ComponentRigidBody> {
    friend class  ComponentTransform;
    friend class  ComponentRigidBody;
    private:

    public:
        SystemSyncRigidToTransform(Engine::priv::ECS&);
};

#endif
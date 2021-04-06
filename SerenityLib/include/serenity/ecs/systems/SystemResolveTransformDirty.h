#pragma once
#ifndef ENGINE_ECS_SYSTEM_RESOLVE_TRANSFORM_DIRTY_H
#define ENGINE_ECS_SYSTEM_RESOLVE_TRANSFORM_DIRTY_H

class  ComponentTransform;
class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemResolveTransformDirty final : public SystemCRTP<SystemResolveTransformDirty, ComponentTransform, ComponentRigidBody> {
    friend class  ComponentTransform;
    friend class  ComponentRigidBody;
    private:

    public:
        SystemResolveTransformDirty(Engine::priv::ECS& ecs);
};

#endif
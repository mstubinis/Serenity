#pragma once
#ifndef ENGINE_ECS_SYSTEM_RIGID_TRANSFORM_SYNC_H
#define ENGINE_ECS_SYSTEM_RIGID_TRANSFORM_SYNC_H

class  ComponentTransform;
class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemRigidTransformSync final : public SystemCRTP<SystemRigidTransformSync, ComponentTransform, ComponentRigidBody> {
    friend class  ComponentTransform;
    friend class  ComponentRigidBody;
    private:

    public:
        SystemRigidTransformSync(Engine::priv::ECS& ecs);
};

#endif
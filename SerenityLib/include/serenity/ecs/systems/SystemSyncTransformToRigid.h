#pragma once
#ifndef ENGINE_ECS_SYSTEM_SYNC_TRANSFORM_TO_RIGID_H
#define ENGINE_ECS_SYSTEM_SYNC_TRANSFORM_TO_RIGID_H

class  ComponentTransform;
class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemSyncTransformToRigid final : public SystemCRTP<SystemSyncTransformToRigid, ComponentTransform, ComponentRigidBody> {
    friend class  ComponentTransform;
    friend class  ComponentRigidBody;
    private:

    public:
        SystemSyncTransformToRigid(Engine::priv::ECS& ecs);
};

#endif
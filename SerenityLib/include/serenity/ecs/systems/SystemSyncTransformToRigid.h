#pragma once
#ifndef ENGINE_ECS_SYSTEM_SYNC_TRANSFORM_TO_RIGID_H
#define ENGINE_ECS_SYSTEM_SYNC_TRANSFORM_TO_RIGID_H

class  ComponentTransform;
class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>

//if the transform component does NOT have a parent, then set it to the rigid body's world matrix, allowing the transform to sync with the physics step simulation earlier on
class SystemSyncTransformToRigid final : public SystemCRTP<SystemSyncTransformToRigid, ComponentTransform, ComponentRigidBody> {
    friend class  ComponentTransform;
    friend class  ComponentRigidBody;
    private:

    public:
        SystemSyncTransformToRigid(Engine::priv::ECS& ecs);
};

#endif
#pragma once
#ifndef ENGINE_ECS_SYSTEM_STEP_PHYSICS_H
#define ENGINE_ECS_SYSTEM_STEP_PHYSICS_H

class  ComponentRigidBody;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemStepPhysics final : public SystemCRTP<SystemStepPhysics, ComponentRigidBody> {
    friend class  ComponentRigidBody;
    private:

    public:
        SystemStepPhysics(Engine::priv::ECS& ecs);
};

#endif
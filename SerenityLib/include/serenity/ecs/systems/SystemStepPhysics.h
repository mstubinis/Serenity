#pragma once
#ifndef ENGINE_ECS_SYSTEM_STEP_PHYSICS_H
#define ENGINE_ECS_SYSTEM_STEP_PHYSICS_H

class  ComponentBodyRigid;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemStepPhysics final : public SystemCRTP<SystemStepPhysics, ComponentBodyRigid> {
    friend class  ComponentBodyRigid;
    private:

    public:
        SystemStepPhysics(Engine::priv::ECS& ecs);
};

#endif
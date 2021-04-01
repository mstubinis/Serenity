#pragma once
#ifndef ENGINE_ECS_SYSTEM_RIGID_TRANSFORM_SYNC_H
#define ENGINE_ECS_SYSTEM_RIGID_TRANSFORM_SYNC_H

class  ComponentBody;
class  ComponentBodyRigid;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemRigidTransformSync final : public SystemCRTP<SystemRigidTransformSync, ComponentBody, ComponentBodyRigid> {
    friend class  ComponentBody;
    friend class  ComponentBodyRigid;
    private:

    public:
        SystemRigidTransformSync(Engine::priv::ECS& ecs);
};

#endif
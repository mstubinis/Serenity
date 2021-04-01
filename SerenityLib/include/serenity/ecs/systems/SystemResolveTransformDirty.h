#pragma once
#ifndef ENGINE_ECS_SYSTEM_RESOLVE_TRANSFORM_DIRTY_H
#define ENGINE_ECS_SYSTEM_RESOLVE_TRANSFORM_DIRTY_H

class  ComponentBody;
class  ComponentBodyRigid;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemResolveTransformDirty final : public SystemCRTP<SystemResolveTransformDirty, ComponentBody, ComponentBodyRigid> {
    friend class  ComponentBody;
    friend class  ComponentBodyRigid;
    private:

    public:
        SystemResolveTransformDirty(Engine::priv::ECS& ecs);
};

#endif
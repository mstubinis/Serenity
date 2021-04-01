#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPOUND_CHILD_TRANSFORMS_H
#define ENGINE_ECS_SYSTEM_COMPOUND_CHILD_TRANSFORMS_H

class  ComponentBody;
class  ComponentCollisionShape;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemCompoundChildTransforms final : public SystemCRTP<SystemCompoundChildTransforms, ComponentBody, ComponentCollisionShape> {
    private:

    public:
        SystemCompoundChildTransforms(Engine::priv::ECS& ecs);
};

#endif
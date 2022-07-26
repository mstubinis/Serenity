#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPOUND_CHILD_TRANSFORMS_H
#define ENGINE_ECS_SYSTEM_COMPOUND_CHILD_TRANSFORMS_H

class  ComponentCollisionShape;

#include <serenity/ecs/systems/SystemBaseClass.h>

//if collision shape is a bullet compound child shape, update the local collision shape matrix to match what was calculated via the parent child heirarchy
class SystemCompoundChildTransforms final : public SystemCRTP<SystemCompoundChildTransforms, ComponentCollisionShape> {
    private:

    public:
        SystemCompoundChildTransforms(Engine::priv::ECS&);
};

#endif
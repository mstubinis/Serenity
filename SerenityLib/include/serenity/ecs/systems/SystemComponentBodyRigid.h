#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_RIGID_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_RIGID_H

class  ComponentBodyRigid;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemComponentBodyRigid final : public SystemCRTP<SystemComponentBodyRigid, ComponentBodyRigid> {
    friend class  ComponentBodyRigid;
public:
    SystemComponentBodyRigid(Engine::priv::ECS& ecs);
};

#endif
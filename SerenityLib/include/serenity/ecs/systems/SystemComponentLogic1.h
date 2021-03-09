#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_LOGIC1_H
#define ENGINE_ECS_SYSTEM_COMPONENT_LOGIC1_H

class  ComponentLogic1;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentLogic1 final : public SystemCRTP<SystemComponentLogic1, ComponentLogic1> {
    private:

    public:
        SystemComponentLogic1(Engine::priv::ECS& ecs);
};

#endif
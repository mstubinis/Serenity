#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_LOGIC2_H
#define ENGINE_ECS_SYSTEM_COMPONENT_LOGIC2_H

class  ComponentLogic2;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentLogic2 final : public SystemCRTP<SystemComponentLogic2, ComponentLogic2> {
    private:

    public:
        SystemComponentLogic2(Engine::priv::ECS& ecs);
};

#endif
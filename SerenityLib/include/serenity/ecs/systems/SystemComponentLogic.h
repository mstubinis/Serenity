#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_LOGIC_H
#define ENGINE_ECS_SYSTEM_COMPONENT_LOGIC_H

class  ComponentLogic;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentLogic final : public SystemCRTP<SystemComponentLogic, ComponentLogic> {
    private:

    public:
        SystemComponentLogic(Engine::priv::ECS&);
};

#endif
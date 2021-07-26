#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_MODEL_H
#define ENGINE_ECS_SYSTEM_COMPONENT_MODEL_H

class  ComponentModel;

#include <serenity/ecs/systems/SystemBaseClass.h>

//recalculates model radius, and process animations on update
class SystemComponentModel final : public SystemCRTP<SystemComponentModel, ComponentModel> {
    private:

    public:
        SystemComponentModel(Engine::priv::ECS& ecs);
};

#endif
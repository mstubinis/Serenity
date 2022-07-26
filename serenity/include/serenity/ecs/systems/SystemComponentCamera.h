#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_CAMERA_H
#define ENGINE_ECS_SYSTEM_COMPONENT_CAMERA_H

class  ComponentCamera;

#include <serenity/ecs/systems/SystemBaseClass.h>

//on update - calculate view frustum planes
class SystemComponentCamera final : public SystemCRTP<SystemComponentCamera, ComponentCamera> {
    private:

    public:
        SystemComponentCamera(Engine::priv::ECS&);
};

#endif
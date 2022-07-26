#pragma once
#ifndef ENGINE_ECS_SYSTEM_SCENE_UPDATE_H
#define ENGINE_ECS_SYSTEM_SCENE_UPDATE_H

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemSceneUpdate final : public SystemCRTP<SystemSceneUpdate> {
    private:

    public:
        SystemSceneUpdate(Engine::priv::ECS&);
};

#endif
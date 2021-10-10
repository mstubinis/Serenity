#pragma once
#ifndef ENGINE_ECS_SYSTEM_SCENE_CHANGING_H
#define ENGINE_ECS_SYSTEM_SCENE_CHANGING_H

#include <serenity/ecs/systems/SystemBaseClass.h>

//on update - if a scene swap occured, process the ecs and systems accordingly
class SystemSceneChanging final : public SystemCRTP<SystemSceneChanging> {
    private:

    public:
        SystemSceneChanging(Engine::priv::ECS&);
};

#endif
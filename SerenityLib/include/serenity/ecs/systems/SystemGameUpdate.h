#pragma once
#ifndef ENGINE_ECS_SYSTEM_GAME_UPDATE_H
#define ENGINE_ECS_SYSTEM_GAME_UPDATE_H

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemGameUpdate final : public SystemCRTP<SystemGameUpdate> {
    private:

    public:
        SystemGameUpdate(Engine::priv::ECS&);
};

#endif
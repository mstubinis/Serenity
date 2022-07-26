#include <serenity/ecs/systems/SystemGameUpdate.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/EngineGameFunctions.h>
#include <serenity/system/Engine.h>

SystemGameUpdate::SystemGameUpdate(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        Game::update(dt, *Engine::priv::Core::m_Engine->m_GameCore);
        Engine::priv::Core::m_Engine->m_LUAModule.update(dt);
    });
}

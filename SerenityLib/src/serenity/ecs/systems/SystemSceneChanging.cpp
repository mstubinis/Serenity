#include <serenity/ecs/systems/SystemSceneChanging.h>
#include <serenity/scene/Scene.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/Engine.h>
#include <serenity/events/Event.h>

SystemSceneChanging::SystemSceneChanging(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& hasSwap = std::get<2>(Engine::priv::Core::m_Engine->m_ResourceManager.m_SceneSwap);
        if (hasSwap == false) {
            return;
        }
        auto& oldScene = std::get<0>(Engine::priv::Core::m_Engine->m_ResourceManager.m_SceneSwap);
        auto& newScene = std::get<1>(Engine::priv::Core::m_Engine->m_ResourceManager.m_SceneSwap);

        Event ev{ EventType::SceneChanged };
        ev.eventSceneChanged = Engine::priv::EventSceneChanged{ oldScene, newScene };
        Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);

        ENGINE_PRODUCTION_LOG("---- Scene Change started (" << ((oldScene) ? (oldScene->name()) : "initial") << ") to (" << newScene->name() << ") ----");
        if (oldScene) {
            Engine::priv::PublicScene::GetECS(*oldScene).onSceneLeft(*oldScene);
        }
        if (newScene) {
            Engine::priv::PublicScene::GetECS(*newScene).onSceneEntered(*newScene);
            newScene->m_SkipRenderThisFrame = true;
            newScene->m_WasJustSwappedTo    = true;
            ENGINE_PRODUCTION_LOG("-------- Scene Change to (" << newScene->name() << ") ended --------");
        }
        hasSwap  = false;
        oldScene = newScene;
        newScene = nullptr;
    });
}

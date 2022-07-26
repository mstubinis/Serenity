#include <serenity/ecs/systems/SystemSceneUpdate.h>
#include <serenity/scene/Scene.h>

SystemSceneUpdate::SystemSceneUpdate(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        scene.m_OnUpdateFunctor(&scene, dt);
    });
}

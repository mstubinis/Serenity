#include <serenity/ecs/systems/SystemComponentScript.h>


SystemComponentScript::SystemComponentScript(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {

    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {

    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {

    });
}

#include <serenity/ecs/ComponentName.h>

#pragma region Component
ComponentName::ComponentName(Entity entity) 
    : m_Owner{ entity }
{}
ComponentName::ComponentName(Entity entity, std::string_view Name)
    : m_Owner{ entity }
    , m_Data{ Name }
{}

#pragma endregion

/*
#pragma region System
ComponentName_System_CI::ComponentName_System_CI() {
    setUpdateFunction([](void* system, void* componentPool, const float dt, Scene& scene) { });
    setOnComponentAddedToEntityFunction([](void* system, void* component, Entity entity) { });
    setOnComponentRemovedFromEntityFunction([](void* system, Entity entity) { });
    setOnEntityAddedToSceneFunction([](void* system, void* componentPool, Entity entity, Scene& scene) { });
    setOnSceneEnteredFunction([](void* system, void* componentPool, Scene& scene) { });
    setOnSceneLeftFunction([](void* system, void* componentPool, Scene& scene) { });
}
#pragma endregion
*/
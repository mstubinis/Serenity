#include <serenity/ecs/components/ComponentName.h>

#pragma region Component
ComponentName::ComponentName(Entity entity) 
{}
ComponentName::ComponentName(Entity entity, std::string_view Name)
    : m_Data { Name }
{}

#pragma endregion

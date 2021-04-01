#include <serenity/ecs/components/ComponentName.h>

#pragma region Component
ComponentName::ComponentName(Entity entity) 
    : m_Owner{ entity }
{}
ComponentName::ComponentName(Entity entity, std::string_view Name)
    : m_Owner{ entity }
    , m_Data { Name }
{}

#pragma endregion

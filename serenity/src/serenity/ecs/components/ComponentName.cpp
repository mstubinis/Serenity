#include <serenity/ecs/components/ComponentName.h>

#pragma region ComponentName
ComponentName::ComponentName(Entity entity) 
{}
ComponentName::ComponentName(Entity entity, std::string_view Name)
    : m_Data { Name }
{}
#pragma endregion


#pragma region ComponentNameLUABinder
const std::string& Engine::priv::ComponentNameLUABinder::name() const {
    return m_Owner.getComponent<ComponentName>()->name();
}
size_t Engine::priv::ComponentNameLUABinder::size() const {
    return m_Owner.getComponent<ComponentName>()->size();
}
bool Engine::priv::ComponentNameLUABinder::empty() const {
    return m_Owner.getComponent<ComponentName>()->empty();
}
#pragma endregion
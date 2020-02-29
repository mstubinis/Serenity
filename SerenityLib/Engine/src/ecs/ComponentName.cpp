#include <ecs/ComponentName.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component
ComponentName::ComponentName(const Entity& entity) {
    m_Owner = entity;
    m_Data = "";
}
ComponentName::ComponentName(const Entity& entity, const string& Name) {
    m_Owner = entity;
    m_Data = Name;
}
ComponentName::ComponentName(const Entity& entity, const char* Name) {
    m_Owner = entity;
    m_Data = Name;
}
ComponentName::~ComponentName() {
    m_Data = "";
}
const string& ComponentName::name() const { 
    return m_Data;
}
const size_t ComponentName::size() const {
    return m_Data.size();
}
const bool ComponentName::empty() const {
    return m_Data.empty();
}
#pragma endregion

#pragma region System

struct priv::ComponentName_UpdateFunction final {void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
}};
struct priv::ComponentName_ComponentAddedToEntityFunction final {void operator()(void* system, void* component, Entity& entity) const {
}};
struct priv::ComponentName_ComponentRemovedFromEntityFunction final { void operator()(void* system, Entity& entity) const {
}};
struct priv::ComponentName_EntityAddedToSceneFunction final {void operator()(void* system, void* componentPool, Entity& entity, Scene& scene) const {
}};
struct priv::ComponentName_SceneEnteredFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {
}};
struct priv::ComponentName_SceneLeftFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {
}};

ComponentName_System_CI::ComponentName_System_CI() {
    setUpdateFunction(ComponentName_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentName_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentName_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentName_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentName_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentName_SceneLeftFunction());
}


#pragma endregion
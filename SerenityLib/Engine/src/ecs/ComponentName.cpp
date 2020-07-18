#include "core/engine/utils/PrecompiledHeader.h"
#include <ecs/ComponentName.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component
ComponentName::ComponentName(Entity entity) {
    m_Owner = entity;
    m_Data = "";
}
ComponentName::ComponentName(Entity entity, const string& Name) {
    m_Owner = entity;
    m_Data = Name;
}
ComponentName::ComponentName(Entity entity, const char* Name) {
    m_Owner = entity;
    m_Data = Name;
}
ComponentName::~ComponentName() {
    m_Data = "";
}
const string& ComponentName::name() const { 
    return m_Data;
}
void ComponentName::setName(const string& name) {
    m_Data = name;
}
void ComponentName::setName(const char* name) {
    m_Data = name;
}
size_t ComponentName::size() const {
    return m_Data.size();
}
bool ComponentName::empty() const {
    return m_Data.empty();
}
#pragma endregion

#pragma region System

ComponentName_System_CI::ComponentName_System_CI() {
    setUpdateFunction([](void* system, void* componentPool, const float dt, Scene& scene) {
    });
    setOnComponentAddedToEntityFunction([](void* system, void* component, Entity entity) {
    });
    setOnComponentRemovedFromEntityFunction([](void* system, Entity entity) {
    });
    setOnEntityAddedToSceneFunction([](void* system, void* componentPool, Entity entity, Scene& scene) {
    });
    setOnSceneEnteredFunction([](void* system, void* componentPool, Scene& scene) {
    });
    setOnSceneLeftFunction([](void* system, void* componentPool, Scene& scene) {
    });
}


#pragma endregion
#include <ecs/ComponentName.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component
ComponentName::ComponentName(const Entity& entity) : ComponentBaseClass(entity){
    m_Data = "";
}
ComponentName::ComponentName(const Entity& entity, const string& Name) : ComponentBaseClass(entity) {
    m_Data = Name;
}
ComponentName::ComponentName(const Entity& entity, const char* Name) : ComponentBaseClass(entity) {
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

struct epriv::ComponentName_UpdateFunction final {void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
}};
struct epriv::ComponentName_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {
}};
struct epriv::ComponentName_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentName_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {
}};
struct epriv::ComponentName_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {
}};

ComponentName_System::ComponentName_System() {
    setUpdateFunction(ComponentName_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentName_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentName_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentName_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentName_SceneLeftFunction());
}


#pragma endregion
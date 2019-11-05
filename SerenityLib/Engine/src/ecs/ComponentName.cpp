#include <ecs/ComponentName.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component
ComponentName::ComponentName(const Entity& _entity) : ComponentBaseClass(_entity){
    _data = "";
}
ComponentName::ComponentName(const Entity& _entity, const string& _name) : ComponentBaseClass(_entity) {
    _data = _name;
}
ComponentName::ComponentName(const Entity& _entity, const char* _name) : ComponentBaseClass(_entity) {
    _data = _name;
}
ComponentName::~ComponentName() {
    _data = "";
}
const string& ComponentName::name() const { 
    return _data; 
}
const size_t ComponentName::size() const {
    return _data.size(); 
}
const bool ComponentName::empty() const {
    return _data.empty();
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
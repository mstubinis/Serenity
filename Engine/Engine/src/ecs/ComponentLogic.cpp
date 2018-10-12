#include "ComponentLogic.h"

using namespace Engine;
using namespace std;

#pragma region Component

ComponentLogic::ComponentLogic(Entity& _entity) : ComponentBaseClass(_entity){
}
ComponentLogic::~ComponentLogic(){
}

void ComponentLogic::call(const float& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogicUpdateFunction final { void operator()(void* _componentPool, const float& dt) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic>*)_componentPool;
    auto& components = pool.pool();
    for (uint i = 0; i < components.size(); ++i) {
        auto& component = components[i];
        component.call(dt);
    }
}};
struct epriv::ComponentLogicComponentAddedToEntityFunction final { void operator()(void* _component) const {
    ComponentLogic& component = *(ComponentLogic*)_component;

}};
struct epriv::ComponentLogicEntityAddedToSceneFunction final { void operator()(void* _componentPool, Entity& _entity) const {
    auto& scene = _entity.scene();
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic>*)_componentPool;
    auto& component = *pool.getComponent(_entity);
}};
struct epriv::ComponentLogicSceneEnteredFunction final { void operator()(void* _componentPool, Scene& _Scene) const {

}};
struct epriv::ComponentLogicSceneLeftFunction final { void operator()(void* _componentPool, Scene& _Scene) const {

}};

ComponentLogicSystem::ComponentLogicSystem() {
    setUpdateFunction(epriv::ComponentLogicUpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentLogicComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentLogicEntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentLogicSceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentLogicSceneLeftFunction());
}

#pragma endregion
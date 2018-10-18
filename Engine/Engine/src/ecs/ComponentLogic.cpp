#include "ComponentLogic.h"

using namespace Engine;
using namespace std;

#pragma region Component

ComponentLogic::~ComponentLogic(){

}

ComponentLogic::ComponentLogic(const ComponentLogic& other){
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
}
ComponentLogic& ComponentLogic::operator=(const ComponentLogic& other){
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    return *this;
}
ComponentLogic::ComponentLogic(ComponentLogic&& other) noexcept{
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    return *this;
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
struct epriv::ComponentLogicComponentAddedToEntityFunction final { void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogicEntityAddedToSceneFunction final { void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

}};
struct epriv::ComponentLogicSceneEnteredFunction final { void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogicSceneLeftFunction final { void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogicSystem::ComponentLogicSystem() {
    setUpdateFunction(epriv::ComponentLogicUpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentLogicComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentLogicEntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentLogicSceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentLogicSceneLeftFunction());
}

#pragma endregion
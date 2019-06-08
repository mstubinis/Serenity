#include <ecs/ComponentLogic.h>

using namespace Engine;
using namespace Engine::epriv;
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


void ComponentLogic::call(const double& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic_UpdateFunction final { void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic>*)_componentPool;
    auto& components = pool.pool();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct epriv::ComponentLogic_ComponentAddedToEntityFunction final { void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic_EntityAddedToSceneFunction final { void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentLogic_SceneEnteredFunction final { void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic_SceneLeftFunction final { void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic_System::ComponentLogic_System() {
    setUpdateFunction(
        ComponentLogic_UpdateFunction());
    setOnComponentAddedToEntityFunction(
        ComponentLogic_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(
        ComponentLogic_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(
        ComponentLogic_SceneEnteredFunction());
    setOnSceneLeftFunction(
        ComponentLogic_SceneLeftFunction());
}

#pragma endregion
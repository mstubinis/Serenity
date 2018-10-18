#include "ComponentLogic1.h"

using namespace Engine;
using namespace std;

#pragma region Component

ComponentLogic1::~ComponentLogic1() {
}

ComponentLogic1::ComponentLogic1(const ComponentLogic1& other) {
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
}
ComponentLogic1& ComponentLogic1::operator=(const ComponentLogic1& other) {
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    return *this;
}
ComponentLogic1::ComponentLogic1(ComponentLogic1&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
}
ComponentLogic1& ComponentLogic1::operator=(ComponentLogic1&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    return *this;
}

void ComponentLogic1::call(const float& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic1UpdateFunction final {void operator()(void* _componentPool, const float& dt) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic1>*)_componentPool;
    auto& components = pool.pool();
    for (uint i = 0; i < components.size(); ++i) {
        auto& component = components[i];
        component.call(dt);
    }
}};
struct epriv::ComponentLogic1ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic1EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

}};
struct epriv::ComponentLogic1SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic1SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic1System::ComponentLogic1System() {
    setUpdateFunction(epriv::ComponentLogic1UpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentLogic1ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentLogic1EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentLogic1SceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentLogic1SceneLeftFunction());
}

#pragma endregion
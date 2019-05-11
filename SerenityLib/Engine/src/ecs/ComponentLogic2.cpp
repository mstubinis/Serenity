#include "ComponentLogic2.h"

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic2::~ComponentLogic2() {
}

ComponentLogic2::ComponentLogic2(const ComponentLogic2& other) {
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
}
ComponentLogic2& ComponentLogic2::operator=(const ComponentLogic2& other) {
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    return *this;
}
ComponentLogic2::ComponentLogic2(ComponentLogic2&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
}
ComponentLogic2& ComponentLogic2::operator=(ComponentLogic2&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    return *this;
}

void ComponentLogic2::call(const float& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic2_UpdateFunction final {void operator()(void* _componentPool, const float& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic2>*)_componentPool;
    auto& components = pool.pool();
    for (uint i = 0; i < components.size(); ++i) {
        auto& component = components[i];
        component.call(dt);
    }
}};
struct epriv::ComponentLogic2_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic2_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

}};
struct epriv::ComponentLogic2_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic2_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic2_System::ComponentLogic2_System() {
    setUpdateFunction(
        ComponentLogic2_UpdateFunction());
    setOnComponentAddedToEntityFunction(
        ComponentLogic2_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(
        ComponentLogic2_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(
        ComponentLogic2_SceneEnteredFunction());
    setOnSceneLeftFunction(
        ComponentLogic2_SceneLeftFunction());
}

#pragma endregion
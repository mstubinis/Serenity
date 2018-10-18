#include "ComponentLogic3.h"

using namespace Engine;
using namespace std;

#pragma region Component

ComponentLogic3::~ComponentLogic3() {
}

ComponentLogic3::ComponentLogic3(const ComponentLogic3& other) {
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
}
ComponentLogic3& ComponentLogic3::operator=(const ComponentLogic3& other) {
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    return *this;
}
ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
}
ComponentLogic3& ComponentLogic3::operator=(ComponentLogic3&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    return *this;
}

void ComponentLogic3::call(const float& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic3UpdateFunction final {void operator()(void* _componentPool, const float& dt) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic3>*)_componentPool;
    auto& components = pool.pool();
    for (uint i = 0; i < components.size(); ++i) {
        auto& component = components[i];
        component.call(dt);
    }
}};
struct epriv::ComponentLogic3ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic3EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

}};
struct epriv::ComponentLogic3SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic3SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic3System::ComponentLogic3System() {
    setUpdateFunction(epriv::ComponentLogic3UpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentLogic3ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentLogic3EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentLogic3SceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentLogic3SceneLeftFunction());
}

#pragma endregion
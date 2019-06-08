#include <ecs/ComponentLogic1.h>

using namespace Engine;
using namespace Engine::epriv;
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

void ComponentLogic1::call(const double& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic1_UpdateFunction final {void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic1>*)_componentPool;
    auto& components = pool.pool();
    for (auto& component: components) {
        component.call(dt);
    }
}};
struct epriv::ComponentLogic1_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic1_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentLogic1_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic1_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic1_System::ComponentLogic1_System() {
    setUpdateFunction(
        ComponentLogic1_UpdateFunction());
    setOnComponentAddedToEntityFunction(
        ComponentLogic1_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(
        ComponentLogic1_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(
        ComponentLogic1_SceneEnteredFunction());
    setOnSceneLeftFunction(
        ComponentLogic1_SceneLeftFunction());
}

#pragma endregion